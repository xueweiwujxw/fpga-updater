package bypass

import spinal.core._
import spinal.lib._

import spinal.lib.bus.amba4.axi._
import spinal.lib.com.spi._

import spinalutils.libs.axispi._
import spinalutils.xilinx.ip._

case class Crossbar(axiconf: Axi4Config) extends Component {
  val io = new Bundle {
    val s_axi = slave(Axi4(axiconf))
    val m_spi = master(SpiMaster())
  }

  val spi_axi_conf = axiconf.copy(addressWidth = 12)
  val spi_axi = Axi4SpiMasterConv(
    config = Axi4SpiMasterConfig(
      axiConfig = spi_axi_conf,
      spiConfig = SpiMasterCtrlGenerics(ssWidth = 1, timerWidth = 8, dataWidth = 8),
      cmdFifoDepth = 1024,
      rspFifoDepth = 1024
    ),
    ipName = "flash"
  )

  spi_axi.io.m_spi <> io.m_spi

  val axicrossbar = Axi4CrossbarFactory()

  axicrossbar.addSlaves(
    spi_axi.io.s_axi -> (0x80090000L, 4 KiB)
  )

  axicrossbar.addConnections(
    io.s_axi -> List(
      spi_axi.io.s_axi
    )
  )

  axicrossbar.build()
}
