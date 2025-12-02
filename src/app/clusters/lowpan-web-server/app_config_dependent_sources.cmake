# Copyright (c) 2025 Lowpan.com
# SPDX-License-Identifier: Proprietary

TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/web-server.cpp"
    "${CLUSTER_DIR}/web-server.h"
)
