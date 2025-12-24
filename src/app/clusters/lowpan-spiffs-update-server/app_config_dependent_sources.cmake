# Copyright (c) 2025 Lowpan.com
# SPDX-License-Identifier: Proprietary

TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/spiffs-update-server.cpp"
    "${CLUSTER_DIR}/spiffs-update-server.h"
)
