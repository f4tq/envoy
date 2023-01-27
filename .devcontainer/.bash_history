$(bazel info bazel-genfiles)/source/exe/envoy-static --config-path /path/to/your/envoy/config.yaml
bazel build --//contrib/golang/filters/http/source:enabled=true
bazel build //contrib/golang/filters/http/test/test_data/echo:filter.so
bazel build //contrib/golang/filters/http/test/test_data/passthrough:filter.so
bazel build --worker_verbose //contrib/golang/filters/http/test/test_data/passthrough:filter.so
sudo apt-get install -y less
bazel test --worker_verbose //contrib/golang/filters/http
bazel test --worker_verbose //contrib/golang/filters/http/test:*
bazel test --worker_verbose //contrib/golang/filters/http/test:golang_integration_test
bazel test --jobs=4 --worker_verbose //contrib/golang/filters/http/test:golang_integration_test
tools/vscode/refresh_compdb.sh
bash -x tools/vscode/refresh_compdb.sh
sudo apt-get update -y
sudo apt-get install -y golang
sudo apt-get install -y less
bazel build --jobs=2 --sandbox_debug //contrib/golang/filters/http/test:golang_filter_test
go mod download
go mod tidy
go version
wget https://go.dev/dl/go1.19.linux-amd64.tar.gz 
tar tzf  go1.19.linux-amd64.tar.gz
mkdir tmp
cd tmp
tar xzf ../go1.19.linux-amd64.tar.gz 
du -hs go/
mv go /usr/local
sudo ln -s go /usr/local/go
bazel build -c opt //source/exe:envoy-static
which bazel
bazel clean
bash -x tools/vscode/refresh_compdb.sh
bazel test --jobs=2 --sandbox_debug --worker_verbose //test/extensions/filters/http/cors:cors_filter_test
bazel test --jobs=2 --verbose_failures --sandbox_debug --worker_verbose //test/extensions/filters/http/cors:cors_filter_test
bazel build -c opt //contrib/golang/filters/http/test:golang_filter_test
bazel build --jobs=2 --verbose_failures --sandbox_debug --worker_verbose  //contrib/golang/filters/http/test:golang_filter_test
bazel build -c opt //source/exe:envoy-static
bazel build --jobs=4 --verbose_failures --worker_verbose  //source/exe:envoy-static
bazel build --jobs=2 --verbose_failures --worker_verbose  //source/exe:envoy-static
ls -l bazel-bin/source/exe/envoy-static
ldd bazel-bin/source/exe/envoy-static
nm -AC bazel-bin/source/exe/envoy-static | grep cors 
bazel build --jobs=2 --verbose_failures --worker_verbose  //source/exe:envoy-static
go mod tidy
find contrib/golang/filters/http/source/go
less .devcontainer/devcontainer.json 
go env
ln -s /workspaces/envoy envoyproxy
bazel test --jobs=2 --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:cors_with_direct_response_filter_test --test_arg="-l trace"
bazel query --jobs=2 --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:* --test_arg="-l trace"
bazel query  --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:* --test_arg="-l trace"
bazel query  --verbose_failures  //test/extensions/filters/http/cors:*
bazel query  //test/extensions/filters/http/cors:*
bazel test --jobs=2 --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:cors_with_direct_response_filter_test --test_arg="-l trace"
bash -x tools/vscode/refresh_compdb.sh
bazel test --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:cors_filter_integration_test --test_arg="-l trace"
bazel test --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:cors_filter_test --test_arg="-l trace"
bazel test --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:cors_filter_test --runs_per_test=2 --test_arg="-l trace"
bazel test --verbose_failures --sandbox_debug --test_output=streamed --worker_verbose //test/extensions/filters/http/cors:cors_with_direct_response_filter_test --runs_per_test=2 --test_arg="-l trace"
bazel test -c dbg  //test/extensions/filters/http/cors:cors_with_direct_response_filter_test  
bazel test -c dbg  //test/extensions/filters/http/cors:cors_with_direct_response_filter_test.dwp
gdb bazel-bin/test/extensions/filters/http/cors/cors_filter_integration_test
bazel clean
bazel test -c dbg  //test/extensions/filters/http/cors:cors_filter_integration_test
bazel run @go_sdk//:bin/go -- version
bazel build --verbose_failures --sandbox_debug --worker_verbose  //contrib/golang/filters/http/test:golang_integration_test
gdb
bazel run @go_sdk//:bin/go -- version
bazel build --verbose_failures --sandbox_debug --worker_verbose  //contrib/golang/filters/http/test:golang_integration_test
git checkout -b adobe-cors-directresp
bazel test -c dbg  //test/extensions/filters/http/cors:cors_filter_integration_test
bazel run @go_sdk//:bin/go -- version
bazel run @go_sdk//:bin/go -- env
bazel run @go_sdk//:bin/go -- mod vendor
bazel run @go_sdk//:bin/go -- get github.com/envoyproxy/envoy/contrib/golang/filters/http/source/go/pkg/http
bazel run @go_sdk//:bin/go -- build --buildmode=c-shared  -v -o libfoo.so .
history -w
env 
pwd
cd ./contrib/golang/filters/http/test/test_data/passthrough/
ls -l
bazel 
bazel query
bazel query help
bazel run @go_sdk//:bin/go -- version
bazel run @go_sdk//:bin/go -- env
ls -l
bazel run @go_sdk//:bin/go -- mod vendor
bazel run @go_sdk//:bin/go -- get github.com/envoyproxy/envoy/contrib/golang/filters/http/source/go/pkg/http
bazel run @go_sdk//:bin/go -- build --buildmode=c-shared  -v -o libfoo.so .
ls -lrt
ls -l
ldd libfoo.so 
ls -l
less go.sum 
find ..
git status ../../..
git status
pwd
cd ../dummy/
ls -l
less plugin.go 
history -w
git branch
git log -2 adobe-cors-directresp
git stash list
go env
bazel run @go_sdk//:bin/go -- env
find /build/go 
bazel run @go_sdk//:bin/go -- env
ls -l
pwd
ls -l /workspaces/envoy/
ls -l /workspaces/
ls -la /workspaces/envoy/.vscode/
bazel run @go_sdk//:bin/go -- env
bazel run @go_sdk//:bin/go -- get github.com/go-delve/delve/cmd/dlv
cd ../
ls
ls -l passthrough/
ls -l 
ls -l echo/
cd passthrough/
bazel run @go_sdk//:bin/go -- get github.com/go-delve/delve/cmd/dlv
ls -l /workspaces/envoy/bazel-out
bazel run @go_sdk//:bin/go -- env
ls -l /workspaces/envoy/bazel-out/
ls -l /workspaces/envoy/bazel-out
bazel run @go_sdk//:bin/go -- version
go version
git branch
git diff
go version
pwd
bazel run @go_sdk//:bin/go --  env
bazel run @go_sdk//:bin/go --  env | grep GOVERSION
bazel run @go_sdk//:bin/go --  env 2> /dev/null | grep GOVERSION
bazel clean
bash -x tools/vscode/refresh_compdb.sh
cd /workspaces/envoy/
bash -x tools/vscode/refresh_compdb.sh
history | wc -l
history -w
bazel run @go_sdk//:bin/go -- version
bazel run @go_sdk//:bin/go -- version| echo
bazel run @go_sdk//:bin/go -- env
ls -l $(bazel info bazel-genfiles)
ls -l $(bazel info bazel-genfiles)/external
cat /build/.config/go/env
ls -l /build/.config/procps/
wget -O- https://go.dev/dl/go1.19.linux-amd64.tar.gz | (cd /usr/local &&  sudo tar xzf - )
ls -l /usr/local/
sudo rm -rf /usr/local/go
ls -l /usr/local/bin/
wget -O- https://go.dev/dl/go1.18.linux-amd64.tar.gz | (cd /usr/local &&  sudo tar xzf - )
ln -s /usr/local/go/bin/go /usr/local/bin/go
sudo ln -s /usr/local/go/bin/go /usr/local/bin/go
go version
sudo ln -s /usr/local/go/bin/gofmt /usr/local/bin/gofmt
top
git status
git checkout contrib/golang/filters/http/test/test_data/passthrough/go.*
rm -rf contrib/golang/filters/http/test/test_data/passthrough/modcache/
