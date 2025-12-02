# Copyright (c) 2025 Lowpan.com
# SPDX-License-Identifier: Proprietary

TARGET_SOURCES(
  ${APP_TARGET}
  PRIVATE
    "${CLUSTER_DIR}/ble-sensor-server.cpp"
    "${CLUSTER_DIR}/ble-sensor-server.h"
)
