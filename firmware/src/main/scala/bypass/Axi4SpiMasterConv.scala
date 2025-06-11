package bypass

import spinal.core._
import spinal.lib._
import spinal.lib.bus.amba4.axi._
import spinal.lib.com.spi._
import spinalutils.xilinx.ip._
import spinalutils.libs.axispi._

case class Axi4SpiMasterConv(config: Axi4SpiMasterConfig, ipName: String, ilamode: Boolean = false)
    extends Component {
  val io = new Bundle {
    val s_axi = slave(Axi4(config.axiConfig))
    val m_spi = master(SpiMaster(config.spiConfig.ssWidth))
    val intr = out(Bool())
  }

  val axi_dwidth = AxiDataWidthAdapter(
    AxiDataWidthAdapterConfig(
      name = ipName,
      addressWidth = io.s_axi.config.addressWidth,
      siDataWidth = io.s_axi.config.dataWidth,
      miDataWidth = 32,
      siIdWidth = io.s_axi.config.idWidth
    )
  )

  val axispi = Axi4SpiMaster(config.copy(axiConfig = axi_dwidth.io.m_axi.config))

  io.s_axi >> axi_dwidth.io.s_axi
  axi_dwidth.io.m_axi >> axispi.io.s_axi
  axi_dwidth.io.s_axi_aclk := this.clockDomain.readClockWire
  axi_dwidth.io.s_axi_aresetn := (if (this.clockDomain.config.resetActiveLevel == LOW)
                                    this.clockDomain.readResetWire
                                  else ~this.clockDomain.readResetWire)

  io.m_spi <> axispi.io.m_spi
  io.intr := axispi.io.intr
}
