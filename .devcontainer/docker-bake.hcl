group "default" {
  targets = ["builder"]
}

################################################################################
// MARK: General targets
################################################################################

target "baser" {
  dockerfile = ".devcontainer/Dockerfile"
  target = "baser"
  tags = ["slicer:baser"]
  pull = false
  no-cache = false
}

target "cacher" {
  inherits   = ["baser"]
  target = "cacher"
  tags = ["slicer:cacher"]
}

target "runner" {
  inherits   = ["baser"]
  target = "runner"
  tags = ["slicer:runner"]
}

target "tester" {
  inherits   = ["runner"]
  target = "tester"
  tags = ["slicer:tester"]
}

target "builder" {
  inherits   = ["tester"]
  target = "builder"
  tags = ["slicer:builder"]
}

################################################################################
// MARK: Development targets 
################################################################################

variable "DEV_FROM_STAGE" {
  default = "builder"
}

target "dever" {
  inherits   = ["builder"]
  args = {
    DEV_FROM_STAGE = "${DEV_FROM_STAGE}",
  }
  target = "dever"
  tags = ["slicer:dever"]
}

target "seeder" {
  inherits   = ["dever"]
  target = "seeder"
  tags = ["slicer:seeder"]
  // no-cache-filter = ["compiler"]
  args = {
    CLEAR_WS_CACHE = null,
    // CLEAR_WS_CACHE = "${timestamp()}",
    SEED_WS_CACHE = null,
    // SEED_WS_CACHE = "${timestamp()}",
  }
}

target "compiler" {
  inherits   = ["seeder"]
  target = "compiler"
  tags = ["slicer:compiler"]
  // no-cache-filter = ["compiler"]
  args = {
    BUST_BUILD_CACHE = null,
    // BUST_BUILD_CACHE = "${timestamp()}",
  }
}

target "validator" {
  inherits   = ["compiler"]
  target = "validator"
  tags = ["slicer:validator"]
  args = {
    BUST_TEST_CACHE = null,
    // BUST_TEST_CACHE = "${timestamp()}",
  }
}

target "dancer" {
  inherits   = ["compiler"]
  target = "dancer"
  tags = ["slicer:dancer"]
}

target "exporter" {
  inherits   = ["dancer"]
  target = "exporter"
  tags = ["slicer:exporter"]
}

################################################################################
// MARK: Production targets
################################################################################

target "shipper" {
  inherits   = ["dancer"]
  target = "shipper"
}

variable "RUNNER" {
  default = "runner"
}

target "releaser" {
  inherits   = ["shipper"]
  target = "releaser"
  tags = ["slicer:releaser"]
  args = {
    SHIP_FROM_STAGE = "${RUNNER}",
  }
}

variable "BUILDER" {
  default = "builder"
}

target "debugger" {
  inherits   = ["shipper"]
  target = "debugger"
  tags = ["slicer:debugger"]
  args = {
    SHIP_FROM_STAGE = "${BUILDER}",
  }
}
