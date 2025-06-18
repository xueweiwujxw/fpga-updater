package updater.top

import spinal.core._
import spinal.lib._
import scopt.OptionParser
import bypass._
import _root_.tools.STARTUPE2

import spinalutils.xilinx.ip._
import spinal.lib.com.spi.SpiMaster
import spinalutils.xilinx.IProject
import spinal.lib.blackbox.xilinx.s7.IBUF


case class Top(
    hash: BigInt = 0,
    stamp: BigInt = 0,
    deviceType: EnumDevice.Value,
    xdmaConf: XdmaConfig,
    targetDirectory: String
) extends Component
    with IProject {
  val io = new Bundle {

    /** pcie */
    /** lane rx tx */
    val pci_exp_txn = out(Bits(xdmaConf.lanes bits))
    val pci_exp_txp = out(Bits(xdmaConf.lanes bits))
    val pci_exp_rxn = in(Bits(xdmaConf.lanes bits))
    val pci_exp_rxp = in(Bits(xdmaConf.lanes bits))

    /** sys clock */
    val pcie_sys_clk_p = in(Bool())
    val pcie_sys_clk_n = in(Bool())
    val pcie_sys_rst_n = in(Bool())

    /** flash spi */
    val m_spi_flash = master(SpiMaster(useSclk = false))

    /** fan control */
    val fan_pwm = out(Bool())
  }

  // implement xdma
  val xdma_inst: Xdmavx690 = xdma(xdmaConf, deviceType)
  xdma_inst.setName("xdma_inst")

  val ibufds = IBUFDS_GTE2()
  val ibuf = IBUF()

  ibufds.I <> io.pcie_sys_clk_p
  ibufds.IB <> io.pcie_sys_clk_n
  ibufds.O <> xdma_inst.io.sys_clk
  ibufds.CEB <> False

  ibuf.I <> io.pcie_sys_rst_n
  ibuf.O <> xdma_inst.io.sys_rst_n

  io.fan_pwm := True

  xdma_inst.io.pci_exp_txn <> io.pci_exp_txn
  xdma_inst.io.pci_exp_txp <> io.pci_exp_txp
  xdma_inst.io.pci_exp_rxn <> io.pci_exp_rxn
  xdma_inst.io.pci_exp_rxp <> io.pci_exp_rxp

  val xdma_domain = new ClockDomain(
    clock = xdma_inst.io.axi_aclk,
    reset = xdma_inst.io.axi_aresetn,
    config = ClockDomainConfig(
      clockEdge = RISING,
      resetKind = SYNC,
      resetActiveLevel = LOW
    )
  )

  val xdma_area = new ClockingArea(xdma_domain) {
    // xdma h2c && c2h
    xdma_inst.io.m_axis_h2c(0).ready := True
    xdma_inst.io.s_axis_c2h(0) << xdma_inst.io.s_axis_c2h(0).getZero

    // axi bypass
    val bar = Crossbar(xdma_inst.io.m_axib.config)
    bar.io.s_axi << xdma_inst.io.m_axib
    io.m_spi_flash.mosi := bar.io.m_spi.mosi
    io.m_spi_flash.ss := bar.io.m_spi.ss
    bar.io.m_spi.miso := io.m_spi_flash.miso

    // startup
    val startup = STARTUPE2()
    startup.CLK := 0
    startup.GSR := 0
    startup.GTS := 0
    startup.KEYCLEARB := 1
    startup.PACK := 1
    startup.USRCCLKO := bar.io.m_spi.sclk.asBits
    startup.USRCCLKTS := 0
    startup.USRDONEO := 1
    startup.USRDONETS := 1

    // interrupt
    xdma_inst.io.usr_irq_req := 0
  }

  noIoPrefix()
  outputTcl(f"${targetDirectory}generate.tcl")
  outputXdc(f"./deps/xdc/base-${deviceType}.xdc", f"${targetDirectory}constraint.xdc")
}

object TopGen extends App {
  case class TopGenParameter(
      hash: String = "0",
      stamp: BigInt = 0,
      deviceType: String = "xc7vx690tffg1158-2",
      pcieLocation: String = "X0Y1"
  )
  def generate(param: TopGenParameter): Unit = {
    val deviceType = EnumDevice.withName(param.deviceType)
    val targetDirectory = "generated/top/"
    SpinalConfig(mode = Verilog, targetDirectory = targetDirectory).generateVerilog(
      Top(
        hash = BigInt(param.hash, 16),
        stamp = param.stamp,
        deviceType = deviceType,
        xdmaConf = XdmaConfig(
          name = "updater",
          advancedMode = true,
          lanes = 8,
          speed = 2.5,
          refclock = "100",
          dataWidth = 128,
          dataClock = 125,
          h2cStreams = 1,
          c2hStreams = 1,
          bypass = true,
          bypassSize = 4,
          bypassScale = EnumScale.Gigabytes,
          idWidth = 4,
          readChannelIds = 64,
          writeChannelIds = 16,
          userIrq = 16,
          interruptType = EnumInterrupt.NONE,
          location = param.pcieLocation
        ),
        targetDirectory = targetDirectory
      )
    )
  }

  val parser = new OptionParser[TopGenParameter]("TopGenParser") {
    opt[String]('h', "hash")
      .action((x, c) => c.copy(hash = x))
      .text("git commit hash value")
    opt[BigInt]('s', "stamp")
      .action((x, c) => c.copy(stamp = x))
      .text("sbt run time stamp")
    opt[String]('t', "deviceType")
      .action((x, c) => c.copy(deviceType = x))
      .text("fpga device type")
    opt[String]('l', "pcieLocation")
      .action((x, c) => c.copy(pcieLocation = x))
      .text("xdma pcie block location")
  }

  parser.parse(args, TopGenParameter()) match {
    case Some(param) => generate(param)
    case None        => println("Configs are wrong")
  }
}
