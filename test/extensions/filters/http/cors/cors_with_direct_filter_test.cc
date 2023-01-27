#include <chrono>
#include <cstdint>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "envoy/common/hashable.h"
#include "envoy/config/route/v3/route.pb.h"
#include "envoy/config/route/v3/route.pb.validate.h"
#include "envoy/config/route/v3/route_components.pb.h"
#include "envoy/server/filter_config.h"
#include "envoy/type/v3/percent.pb.h"

#include "source/common/config/metadata.h"
#include "source/common/config/well_known_names.h"
#include "source/common/http/header_map_impl.h"
#include "source/common/http/headers.h"
#include "source/common/network/address_impl.h"
#include "source/common/router/config_impl.h"
#include "source/common/router/string_accessor_impl.h"
#include "source/common/stream_info/filter_state_impl.h"

#include "source/extensions/filters/http/cors/cors_filter.h"

#include "test/common/router/route_fuzz.pb.h"
#include "test/extensions/filters/http/common/empty_http_filter_config.h"
#include "test/fuzz/utility.h"
#include "test/mocks/router/mocks.h"
#include "test/mocks/server/instance.h"
#include "test/mocks/upstream/retry_priority.h"
#include "test/mocks/upstream/retry_priority_factory.h"
#include "test/mocks/upstream/test_retry_host_predicate_factory.h"
#include "test/test_common/environment.h"
#include "test/test_common/printers.h"
#include "test/test_common/registry.h"
#include "test/test_common/test_runtime.h"
#include "test/test_common/utility.h"

#include "test/mocks/buffer/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/stats/mocks.h"
#include "test/test_common/printers.h"

#include "test/mocks/buffer/mocks.h"
#include "test/mocks/http/mocks.h"
#include "test/mocks/stats/mocks.h"
#include "test/test_common/printers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::ContainerEq;
using testing::ElementsAre;
using testing::Eq;
using testing::MockFunction;
using testing::NiceMock;
using testing::Pair;
using testing::Return;
using testing::ReturnRef;

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Cors {
namespace {

Matchers::StringMatcherPtr makeExactStringMatcher(const std::string& exact_match) {
  envoy::type::matcher::v3::StringMatcher config;
  config.set_exact(exact_match);
  return std::make_unique<Matchers::StringMatcherImpl<envoy::type::matcher::v3::StringMatcher>>(
      config);
}
} // namespace

struct OptionalGenHeadersArg {
  std::string scheme = "http";
  absl::optional<std::pair<std::string, std::string>> random_value_pair;
};

class ConfigImplTestBase {

protected:
  ConfigImplTestBase()
      : api_(Api::createApiForTest()), engine_(std::make_unique<Regex::GoogleReEngine>()) {
    ON_CALL(factory_context_, api()).WillByDefault(ReturnRef(*api_));
  }

  std::string virtualHostName(const Router::RouteEntry* route) {
    Stats::StatName name = route->virtualHost().statName();
    return factory_context_.scope().symbolTable().toString(name);
  }

  std::string virtualClusterName(const Router::RouteEntry* route,
                                 Http::TestRequestHeaderMapImpl& headers) {
    Stats::StatName name = route->virtualCluster(headers)->statName();
    return factory_context_.scope().symbolTable().toString(name);
  }
  static void checkPathMatchCriterion(const Router::Route* route,
                                      const std::string& expected_matcher,
                                      Router::PathMatchType expected_type) {
    ASSERT_NE(nullptr, route);
    const auto route_entry = route->routeEntry();
    ASSERT_NE(nullptr, route_entry);
    const auto& match_criterion = route_entry->pathMatchCriterion();
    EXPECT_EQ(expected_matcher, match_criterion.matcher());
    EXPECT_EQ(expected_type, match_criterion.matchType());
  }
  // Loads a V3 RouteConfiguration yaml
  static envoy::config::route::v3::RouteConfiguration
  parseRouteConfigurationFromYaml(const std::string& yaml) {
    envoy::config::route::v3::RouteConfiguration route_config;
    TestUtility::loadFromYaml(yaml, route_config);
    TestUtility::validate(route_config);
    return route_config;
  }
  static Http::TestRequestHeaderMapImpl
  genHeaders(const std::string& host, const std::string& path, const std::string& method,
             const OptionalGenHeadersArg& optional_args = {}) {
    return genHeaders(host, path, method, optional_args.scheme, optional_args.random_value_pair);
  }

  static Http::TestRequestHeaderMapImpl
  genHeaders(const std::string& host, const std::string& path, const std::string& method,
             const std::string& scheme,
             absl::optional<std::pair<std::string, std::string>> random_value_pair) {
    auto hdrs =
        Http::TestRequestHeaderMapImpl{{":authority", host},         {":path", path},
                                       {":method", method},          {"x-safe", "safe"},
                                       {"x-global-nope", "global"},  {"x-vhost-nope", "vhost"},
                                       {"x-route-nope", "route"},    {":scheme", scheme},
                                       {"x-forwarded-proto", scheme}};

    if (scheme.empty()) {
      hdrs.remove(":scheme");
    }

    if (random_value_pair.has_value()) {
      hdrs.setByKey(Envoy::Http::LowerCaseString(random_value_pair.value().first),
                    random_value_pair.value().second);
    }
    return hdrs;
  }

  Stats::TestUtil::TestSymbolTable symbol_table_;
  Api::ApiPtr api_;
  NiceMock<Server::Configuration::MockServerFactoryContext> factory_context_;
  Event::SimulatedTimeSystem test_time_;
  ScopedInjectableLoader<Regex::Engine> engine_;
};

class TestConfigImpl : public Router::ConfigImpl {
public:
  TestConfigImpl(
      const envoy::config::route::v3::RouteConfiguration& config,
      Server::Configuration::ServerFactoryContext& factory_context, bool validate_clusters_default,
      const Router::OptionalHttpFilters& optional_http_filters = Router::OptionalHttpFilters())
      : Router::ConfigImpl(config, optional_http_filters, factory_context,
                           ProtobufMessage::getNullValidationVisitor(), validate_clusters_default),
        config_(config) {}

  void setupRouteConfig(const Http::RequestHeaderMap& headers, uint64_t random_value) const {
    ENVOY_LOG_MISC(debug, "setupRouteConfig {} to {}", headers, random_value);
  }

  Router::RouteConstSharedPtr route(const Http::RequestHeaderMap& headers,
                                    const Envoy::StreamInfo::StreamInfo& stream_info,
                                    uint64_t random_value) const override {

    ENVOY_LOG_MISC(debug, "TestConfigImpl route 3 headers: {}  stream_info chainName: {}  routeName: {} ", headers, stream_info.filterChainName(), stream_info.getRouteName());
    return Router::ConfigImpl::route(headers, stream_info, random_value);
  }

  Router::RouteConstSharedPtr route(const Router::RouteCallback& cb,
                                    const Http::RequestHeaderMap& headers,
                                    const StreamInfo::StreamInfo& stream_info,
                                    uint64_t random_value) const override {

  //  setupRouteConfig(headers, random_value);
    ENVOY_LOG_MISC(debug, "TestConfigImpl route 4  headers: {} stream_info: chainName: {}  routeName: {}",  headers, stream_info.filterChainName(), stream_info.getRouteName());

    return Router::ConfigImpl::route(cb, headers, stream_info, random_value);
  }

  Router::RouteConstSharedPtr route(const Router::RouteCallback& cb,
                                    const Http::RequestHeaderMap& headers) const {

    ENVOY_LOG_MISC(debug, "TestConfigImpl route 2  headers: {} ",   headers);

    return route(cb, headers, NiceMock<Envoy::StreamInfo::MockStreamInfo>(), 0);
  }

  Router::RouteConstSharedPtr route(const Http::RequestHeaderMap& headers,
                                    uint64_t random_value) const {
    return route(headers, NiceMock<Envoy::StreamInfo::MockStreamInfo>(), random_value);
  }

  const envoy::config::route::v3::RouteConfiguration config_;
};

class RouteMatcherTest : public testing::Test,
                         public ConfigImplTestBase,
                         public TestScopedRuntime {};

class RouteConfigurationDirectResponseBodyTest : public testing::Test, public ConfigImplTestBase {
public:
  /**
   * @brief  Generate headers for testing
   * @param ssl set true to insert "":scheme: https", else ":scheme http"
   * @param internal nullopt for no such "x-envoy-internal" header, or explicit "true/false"
   * @return Http::TestRequestHeaderMapImpl
   */
  static Http::TestRequestHeaderMapImpl genRedirectHeaders(const std::string& host,
                                                           const std::string& path, bool ssl,
                                                           absl::optional<bool> internal) {
    std::string scheme = ssl ? "https" : "http";
    Http::TestRequestHeaderMapImpl headers{
        {":authority", host}, {":path", path}, {":scheme", scheme}, {"x-forwarded-proto", scheme}};
    if (internal.has_value()) {
      headers.addCopy("x-envoy-internal", internal.value() ? "true" : "false");
    }

    return headers;
  }
};

TEST_F(RouteConfigurationDirectResponseBodyTest, DirectResponseBodyLargerThanDefault) {
  // Set the inline direct response body size to be larger than 4K.
  std::string response_body(2 * 4096, 'A');

  std::unique_ptr<Router::TestCorsPolicy> cors_policy_;
  cors_policy_ = std::make_unique<Router::TestCorsPolicy>();
  cors_policy_->enabled_ = true;
  cors_policy_->shadow_enabled_ = false;
  cors_policy_->allow_origins_.emplace_back(makeExactStringMatcher("*"));
  cors_policy_->allow_methods_ = "GET";
  cors_policy_->allow_headers_ = "content-type";
  cors_policy_->expose_headers_ = "content-type";
  cors_policy_->allow_credentials_ = false;
  cors_policy_->allow_private_network_access_ = true;
  cors_policy_->max_age_ = "0";

  NiceMock<Http::MockStreamDecoderFilterCallbacks> decoder_callbacks_;
  NiceMock<Http::MockStreamEncoderFilterCallbacks> encoder_callbacks_;
  Stats::TestUtil::TestStore stats_;
  CorsFilterConfigSharedPtr config2(new CorsFilterConfig("test.", *stats_.rootScope()));
  CorsFilter filter(config2);
  Router::MockDirectResponseEntry direct_response_entry_;

  filter.setDecoderFilterCallbacks(decoder_callbacks_);
  filter.setEncoderFilterCallbacks(encoder_callbacks_);

  Buffer::OwnedImpl data_;
  Http::TestRequestHeaderMapImpl request_headers_;
  Http::TestRequestTrailerMapImpl request_trailers_;
  Http::TestResponseHeaderMapImpl response_headers_;
  Http::TestResponseTrailerMapImpl response_trailers_;

  // std::string scheme = ssl ? "https" : "http";

  Http::TestRequestHeaderMapImpl request_headers{{":method", "OPTIONS"},
                                                 {"origin", "localhost"},
                                                 {"access-control-request-method", "GET"},
                                                 {":authority", "direct.example.com"},
                                                 {":path", "/"},
                                                 {":scheme", "https"},
                                                 {"x-forwarded-proto", "https"}};

  ON_CALL(*decoder_callbacks_.route_, directResponseEntry())
      .WillByDefault(Return(&direct_response_entry_));

  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter.decodeHeaders(request_headers, false));
  // EXPECT_EQ(true, IsCorsRequest());
  EXPECT_EQ(Http::FilterDataStatus::Continue, filter.decodeData(data_, false));
  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter.decodeTrailers(request_trailers_));

  Http::TestResponseHeaderMapImpl response_headers{};
  EXPECT_EQ(Http::FilterHeadersStatus::Continue, filter.encodeHeaders(response_headers, false));
  EXPECT_EQ("localhost", response_headers.get_("access-control-allow-origin"));

  EXPECT_EQ(Http::FilterDataStatus::Continue, filter.encodeData(data_, false));
  EXPECT_EQ(Http::FilterTrailersStatus::Continue, filter.encodeTrailers(response_trailers_));

}

TEST_F(RouteConfigurationDirectResponseBodyTest, FooBar) {
  std::string response_body(2 * 4096, 'A');

  const std::string yaml = R"EOF(
max_direct_response_body_size_bytes: 8192
virtual_hosts:

  - name: direct
    domains: [direct.example.com]
    cors:
      allow_origin_string_match:
      - safe_regex:
          regex: \*
      allow_methods: "GET"
      filter_enabled:
        default_value:
          numerator: 100
          denominator: HUNDRED
        runtime_key: cors.www.enabled
      shadow_enabled:
        default_value:
          numerator: 0
          denominator: HUNDRED
        runtime_key: cors.www.shadow_enabled

    routes:
      - match: { prefix: "/foo/bar" }
        route:
          cluster: foo_bar
      - match: { prefix: "/" }
        route:
          cluster: default

      - match: { prefix: "/direct"}
        direct_response:
          status: 200
          body:
            inline_string: )EOF" +
                           response_body + "\n";

  factory_context_.cluster_manager_.initializeClusters({"foo_bar", "default"}, {});
  TestConfigImpl config(parseRouteConfigurationFromYaml(yaml), factory_context_, true);

  Http::TestRequestHeaderMapImpl request_headers{{":method", "OPTIONS"},
                                                 {"origin", "localhost"},
                                                 {"access-control-request-method", "GET"},
                                                 {":authority", "direct.example.com"},
                                                 {":path", "/direct"},
                                                 {":scheme", "https"},
                                                 {"x-forwarded-proto", "https"}};

  const auto* direct_response = config.route(request_headers, 0)->directResponseEntry();
  EXPECT_NE(nullptr, direct_response);
  EXPECT_EQ(Http::Code::OK, direct_response->responseCode());
  EXPECT_STREQ("content", direct_response->responseBody().c_str());

  std::vector<std::string> clusters{"default", "foo", "foo_bar", "foo_bar_baz"};

  Router::RouteConstSharedPtr accepted_route = config.route(
      [&clusters](Router::RouteConstSharedPtr route,
                  Router::RouteEvalStatus route_eval_status) -> Router::RouteMatchStatus {
        EXPECT_FALSE(clusters.empty());
        EXPECT_EQ(clusters[clusters.size() - 1], route->routeEntry()->clusterName());
        clusters.pop_back();

        if (clusters.empty()) {
          EXPECT_EQ(route_eval_status, Router::RouteEvalStatus::NoMoreRoutes);
        } else {
          EXPECT_EQ(route_eval_status, Router::RouteEvalStatus::HasMoreRoutes);
        }
        // Returning continue when no more routes are available will be ignored by
        // ConfigImpl::route
        return Router::RouteMatchStatus::Continue;
      },
      genHeaders("bat.com", "/foo/bar/baz", "GET"));
  EXPECT_EQ(accepted_route, nullptr);
    EXPECT_EQ(Http::Code::OK,
            config.route(request_headers, 0)->directResponseEntry()->responseCode());
  EXPECT_EQ(response_body, config.route(request_headers, 0)->directResponseEntry()->responseBody());

}

} // namespace Cors
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
