# Copyright (c) 2025 Lowpan.com
# SPDX-License-Identifier: Proprietary

TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/location-server.cpp"
    "${CLUSTER_DIR}/location-server.h"
)
