#include "halvoePioSPI.hpp"

#include <cstring>

const pin_size_t HALVOE_SPI_PIN_RX = 10;
const pin_size_t HALVOE_SPI_PIN_SCK = 11;
const size_t RX_FIFO_SIZE_BYTES = 8;
int stateMachine = -1;

const size_t IS_READY_PIN = 5;
const size_t FRAME_HEIGHT = 240;
const size_t FRAME_WIDTH = 320;
const size_t FRAME_SIZE = FRAME_HEIGHT * FRAME_WIDTH;

const size_t dmaBufferSize = FRAME_SIZE;
uint8_t dmaBuffer[dmaBufferSize];
bool isDmaFull = false;

void handleDmaIrq()
{
  isDmaFull = true;
  clear_dma_interrupt_request();
  restart_dma_channel(dmaBuffer);
  Serial.println("handleDmaIrq() done");
}

void setup()
{
  Serial.begin(460800);
  while (not Serial) { delay(100); }
  Serial.println("Serial is ready.");

  std::memset(dmaBuffer, 2, dmaBufferSize);
  stateMachine = setupPioSPI(pio1, HALVOE_SPI_PIN_RX, dmaBuffer, dmaBufferSize, handleDmaIrq);
  Serial.println("Leaving setup()");

  pinMode(IS_READY_PIN, OUTPUT);
  digitalWrite(IS_READY_PIN, HIGH);
}

void loop()
{
  // nothing to do here
  if (isDmaFull)
  {
    String printString;

    for (size_t index = 0; index < dmaBufferSize; ++index)
    {
      if (dmaBuffer[index] < 10) { printString.concat('0'); }
      printString.concat(String(dmaBuffer[index], HEX));

      if ((index + 1) % 64 == 0)
      {
        printString.concat('\n');
      }
    }

    Serial.println(printString);

    isDmaFull = false;
  }
}
