ThisBuild / version := "1.1.0"
ThisBuild / scalaVersion := "2.12.18"
ThisBuild / organization := "com.github.xueweiwujxw"

val spinalVersion = "1.12.2"
val spinalCore = "com.github.spinalhdl" %% "spinalhdl-core" % spinalVersion
val spinalLib = "com.github.spinalhdl" %% "spinalhdl-lib" % spinalVersion
val spinalIdslPlugin = compilerPlugin(
  "com.github.spinalhdl" %% "spinalhdl-idsl-plugin" % spinalVersion
)
val spinalUtils = "io.github.xueweiwujxw" %% "spinalutils" % "0.1.4"
val scoptlib = "com.github.scopt" %% "scopt" % "4.1.0"

lazy val fpgaUpdaterLib = (project in file("."))
  .enablePlugins(BuildInfoPlugin)
  .settings(
    name := "FpgaUpdater",
    buildInfoKeys := Seq[BuildInfoKey](name, version, scalaVersion, sbtVersion, organization),
    buildInfoPackage := "FpgaUpdater",
    libraryDependencies ++= Seq(
      spinalCore,
      spinalLib,
      spinalIdslPlugin,
      spinalUtils,
      scoptlib
    )
  )

fork := true
