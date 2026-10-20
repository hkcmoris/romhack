# ROM hack identity. GitHub Actions supplies ROM_VERSION for each CI run.
ROM_BASENAME ?= oraculum
ROM_VERSION ?= 0.2.0-local

# Enable LTO when making a release build. Disable by setting to 0.
USE_LTO_ON_RELEASE ?= 1
