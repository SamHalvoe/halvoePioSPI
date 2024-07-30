#include "halvoePioSPI.hpp"

#include <cstring>

const pin_size_t HALVOE_SPI_PIN_RX = 10;
const pin_size_t HALVOE_SPI_PIN_SCK = 11;
const size_t RX_FIFO_SIZE_BYTES = 8;
int stateMachine = -1;
const size_t dmaBufferSize = 128;
uint8_t dmaBuffer[dmaBufferSize];
bool isDmaFull = false;

void handleDmaIrq()
{
  Serial.println("dma irq begin");
  isDmaFull = true;
  clear_dma_interrupt_request();
  restart_dma_channel();
  Serial.println("dma irq end");
}

void setup()
{
  Serial.begin(115200);
  while (not Serial) { delay(100); }
  Serial.println("Serial is ready.");

  std::memset(dmaBuffer, 2, dmaBufferSize);
  stateMachine = setupPioSPI(pio1, HALVOE_SPI_PIN_RX, dmaBuffer, dmaBufferSize, handleDmaIrq);
  Serial.println("Leaving setup()");
}

void loop()
{
  // nothing to do here
  if (isDmaFull)
  {
    for (size_t index = 0; index < dmaBufferSize; ++index)
    {
      Serial.print(dmaBuffer[index]);

      if ((index + 1) % 4 == 0)
      {
        Serial.println();
      }
      else
      {
        Serial.print(", ");
      }
    }

    isDmaFull = false;
  }
}
