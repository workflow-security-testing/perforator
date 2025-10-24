#!/usr/bin/env bash

set -euo pipefail

# set all secrets

{{ range .Secrets }}
export {{ .Name }}='{{ .Value }}'
{{ end }}

set -x

# set all variables

{{ range .Variables }}
export {{ .Name }}='{{ .Value }}'
{{ end }}

# call job script

set +e
bash /home/builder/job.sh

echo $? > /home/builder/job-exit-code
set -e

