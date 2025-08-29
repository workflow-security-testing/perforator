#!/usr/bin/env bash

set -euxo pipefail

mkdir ~/src

df

(cd ~/src && tar xf ~/code.tgz)


if [[ "${CACHE_RW:-false}" == "false" ]]; then
    BAZEL_PUT_ARGS=""
else
    BAZEL_PUT_ARGS="--bazel-remote-put --bazel-remote-username=${BAZEL_CACHE_USER} --bazel-remote-password-file=${BAZEL_CACHE_PASSWORD_PATH}"
fi

(cd ~/src && ./ya test -T -DCI=github -DCONSISTENT_BUILD=yes -DCONSISTENT_DEBUG=yes --bazel-remote-store --bazel-remote-base-uri=${BAZEL_URI} ${BAZEL_PUT_ARGS} ./perforator)

df
