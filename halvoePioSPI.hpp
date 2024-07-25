#pragma once

#include <hardware/pio.h>
#include <hardware/dma.h>

#include "halvoePioSPI.pio.hpp"

uint setupPioSPI(PIO io_pio, uint in_dataPin)
{
  uint programMemoryOffset = pio_add_program(io_pio, &halvoe_spi_peripheral_program);
  uint stateMachine = pio_claim_unused_sm(io_pio, true);
  halvoe_spi_peripheral_program_init(io_pio, stateMachine, programMemoryOffset, in_dataPin);
  return stateMachine;
}