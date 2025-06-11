package tools

import spinal.core._

case class STARTUPE2(
    PROG_USR: String = "FALSE",
    SIM_CCLK_FREQ: Double = 0.0
) extends BlackBox {
  addGeneric("PROG_USR", PROG_USR)
  addGeneric("SIM_CCLK_FREQ", SIM_CCLK_FREQ)

  /** Configuration main clock output */
  val CFGCLK = out(Bits(1 bits))

  /** Configuration internal oscillator clock output */
  val CFGMCLK = out(Bits(1 bits))

  /** Active high output signal indicating the End Of Startup. */
  val EOS = out(Bits(1 bits))

  /** PROGRAM request to fabric output */
  val PREQ = out(Bits(1 bits))

  /** User start-up clock input */
  val CLK = in(Bits(1 bits))

  /** Global Set/Reset input (GSR cannot be used for the port name) */
  val GSR = in(Bits(1 bits))

  /** Global 3-state input (GTS cannot be used for the port name) */
  val GTS = in(Bits(1 bits))

  /** Clear AES Decrypter Key input from Battery-Backed RAM (BBRAM) */
  val KEYCLEARB = in(Bits(1 bits))

  /** PROGRAM acknowledge input */
  val PACK = in(Bits(1 bits))

  /** User CCLK input For Zynq-7000 devices, this input must be tied to GND */
  val USRCCLKO = in(Bits(1 bits))

  /** User CCLK 3-state enable input For Zynq-7000 devices, this input must be tied to VCC */
  val USRCCLKTS = in(Bits(1 bits))

  /** User DONE pin output control */
  val USRDONEO = in(Bits(1 bits))

  /** User DONE 3-state enable output */
  val USRDONETS = in(Bits(1 bits))
}

case class STARTUPE3(
    PROG_USR: String = "FALSE",
    SIM_CCLK_FREQ: Double = 0.0
) extends BlackBox {
  addGeneric("PROG_USR", PROG_USR)
  addGeneric("SIM_CCLK_FREQ", SIM_CCLK_FREQ)

  /** Configuration main clock output. */
  val CFGCLK = out(Bits(1 bits))

  /** Configuration internal oscillator clock output. */
  val CFGMCLK = out(Bits(1 bits))

  /** Allow receiving on the D input pin. */
  val DI = out(Bits(4 bits))

  /** Active-High output signal indicating the End Of Startup. */
  val EOS = out(Bits(1 bits))

  /** PROGRAM request to fabric output. */
  val PREQ = out(Bits(1 bits))

  /** Allows control of the D pin output. */
  val DO = in(Bits(4 bits))

  /** Allows tristate of the D pin. */
  val DTS = in(Bits(4 bits))

  /** Controls the FCS_B pin for flash access. */
  val FCSBO = in(Bits(1 bits))

  /** Tristate the FCS_B pin. */
  val FCSBTS = in(Bits(1 bits))

  /** Global Set/Reset input (GSR cannot be used for the port). */
  val GSR = in(Bits(1 bits))

  /** Global 3-state input (GTS cannot be used for the port name). */
  val GTS = in(Bits(1 bits))

  /** Clear AES Decrypter Key input from Battery-Backed RAM (BBRAM). */
  val KEYCLEARB = in(Bits(1 bits))

  /** PROGRAM acknowledge input. */
  val PACK = in(Bits(1 bits))

  /** User CCLK input. */
  val USRCCLKO = in(Bits(1 bits))

  /** User CCLK 3-state enable input. */
  val USRCCLKTS = in(Bits(1 bits))

  /** User DONE pin output control. */
  val USRDONEO = in(Bits(1 bits))

  /** User DONE 3-state enable output. */
  val USRDONETS = in(Bits(1 bits))
}
