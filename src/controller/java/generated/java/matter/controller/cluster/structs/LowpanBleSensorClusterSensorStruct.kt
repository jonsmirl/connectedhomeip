/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class LowpanBleSensorClusterSensorStruct(
  val macAddress: String,
  val count: UInt,
  val rssi: UShort,
  val bridged: Boolean
) {
  override fun toString(): String = buildString {
    append("LowpanBleSensorClusterSensorStruct {\n")
    append("\tmacAddress : $macAddress\n")
    append("\tcount : $count\n")
    append("\trssi : $rssi\n")
    append("\tbridged : $bridged\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MAC_ADDRESS), macAddress)
      put(ContextSpecificTag(TAG_COUNT), count)
      put(ContextSpecificTag(TAG_RSSI), rssi)
      put(ContextSpecificTag(TAG_BRIDGED), bridged)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MAC_ADDRESS = 1
    private const val TAG_COUNT = 2
    private const val TAG_RSSI = 3
    private const val TAG_BRIDGED = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): LowpanBleSensorClusterSensorStruct {
      tlvReader.enterStructure(tlvTag)
      val macAddress = tlvReader.getString(ContextSpecificTag(TAG_MAC_ADDRESS))
      val count = tlvReader.getUInt(ContextSpecificTag(TAG_COUNT))
      val rssi = tlvReader.getUShort(ContextSpecificTag(TAG_RSSI))
      val bridged = tlvReader.getBoolean(ContextSpecificTag(TAG_BRIDGED))
      
      tlvReader.exitContainer()

      return LowpanBleSensorClusterSensorStruct(macAddress, count, rssi, bridged)
    }
  }
}
