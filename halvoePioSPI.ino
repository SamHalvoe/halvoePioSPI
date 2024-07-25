#include "halvoePioSPI.hpp"

const pin_size_t HALVOE_SPI_PIN_RX = 10;
const pin_size_t HALVOE_SPI_PIN_SCK = 11;
const size_t RX_FIFO_SIZE_BYTES = 8;
int stateMachine = -1;


void setup()
{
  Serial.begin(115200);
  while (not Serial) { delay(100); }
  Serial.println("Serial is ready.");

  stateMachine = setupPioSPI(pio1, HALVOE_SPI_PIN_RX);
  Serial.println("Leaving setup()");
}

void loop()
{
  if (pio_sm_is_rx_fifo_full(pio1, stateMachine))
  {
    Serial.println("rx_fifo is full");

    for (size_t index = 0; index < RX_FIFO_SIZE_BYTES; ++index)
    {
      Serial.print(pio_sm_get_blocking(pio1, stateMachine));
      Serial.print(", ");
    }

    Serial.println();
  }
}
