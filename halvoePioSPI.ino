#include "halvoePioSPI.hpp"

#include <array>

const pin_size_t HALVOE_SPI_PIN_RX = 10;
const pin_size_t HALVOE_SPI_PIN_SCK = 11;
const pin_size_t HALVOE_SPI_PIN_CS = 12;
const size_t RX_FIFO_SIZE_BYTES = 8;
int stateMachine = -1;

const size_t IS_READY_PIN = 5;
const size_t FRAME_HEIGHT = 240;
const size_t FRAME_WIDTH = 320;
const size_t FRAME_SIZE = FRAME_HEIGHT * FRAME_WIDTH;

const int16_t PIN_STATUS_NOT_YET_READ = -1;
int16_t currentPinStatus = PIN_STATUS_NOT_YET_READ;
int16_t previousPinStatus = PIN_STATUS_NOT_YET_READ;

const size_t dmaBufferSize = 16384; // in bytes (uint8_t)
const size_t dmaBufferCount = 2;
using DmaBuffer = std::array<uint8_t, dmaBufferSize>;
using DmaBufferArray = std::array<DmaBuffer, dmaBufferCount>;
DmaBufferArray dmaBufferArray;
size_t currentDmaBufferIndex = 0;
size_t previousDmaBufferIndex = currentDmaBufferIndex;

DmaBuffer& getCurrentDmaBuffer()
{
  return dmaBufferArray[currentDmaBufferIndex];
}

DmaBuffer& getPreviousDmaBuffer()
{
  return dmaBufferArray[previousDmaBufferIndex];
}

DmaBuffer& getNextDmaBuffer()
{
  previousDmaBufferIndex = currentDmaBufferIndex;
  ++currentDmaBufferIndex;
  if (currentDmaBufferIndex == dmaBufferArray.size()) { currentDmaBufferIndex = 0; }
  return dmaBufferArray[currentDmaBufferIndex];
}

void __time_critical_func(handleDmaIrq)()
{
  halvoePioSPI::clear_dma_interrupt_request();
  //halvoePioSPI::restart_dma_channel(dmaBuffer, dmaBufferSize);
  Serial.println("handleDmaIrq() done");
}

volatile bool isCommandCompleted = false;

void __time_critical_func(handleCSIrq)()
{
  currentPinStatus = digitalReadFast(HALVOE_SPI_PIN_CS) ? HIGH : LOW;
  Serial.println(currentPinStatus);
  Serial.println(previousPinStatus);
  if (currentPinStatus == HIGH && previousPinStatus == LOW)
  {
    isCommandCompleted = true;
    halvoePioSPI::restart_dma_channel(getNextDmaBuffer().data(), dmaBufferSize);
  }

  previousPinStatus = currentPinStatus;
  
  Serial.println("handleCSIrq() done");
}

void setup()
{
  Serial.begin(460800);
  while (not Serial) { delay(100); }
  Serial.println("Serial is ready.");

  for (auto& dmaBuffer: dmaBufferArray)
  {
    dmaBuffer.fill(0);
  }
  
  pinMode(HALVOE_SPI_PIN_CS, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALVOE_SPI_PIN_CS), handleCSIrq, CHANGE);
  stateMachine = halvoePioSPI::setup(pio1, HALVOE_SPI_PIN_RX, getCurrentDmaBuffer().data(), dmaBufferSize, handleDmaIrq);
  Serial.println("Leaving setup()");

  pinMode(IS_READY_PIN, OUTPUT);
  digitalWrite(IS_READY_PIN, HIGH);
}

void loop()
{
  if (isCommandCompleted)
  {
    String printString;

    for (size_t index = 0; index < 128/*dmaBufferSize*/; ++index)
    {
      if (getCurrentDmaBuffer()[index] < 10) { printString.concat('_'); }
      printString.concat(String(getCurrentDmaBuffer()[index], HEX));

      if ((index + 1) % 64 == 0)
      {
        printString.concat('\n');
      }
    }

    for (size_t index = 0; index < 128/*dmaBufferSize*/; ++index)
    {
      if (getPreviousDmaBuffer()[index] < 10) { printString.concat('_'); }
      printString.concat(String(getPreviousDmaBuffer()[index], HEX));

      if ((index + 1) % 64 == 0)
      {
        printString.concat('\n');
      }
    }

    Serial.println(printString);

    isCommandCompleted = false;
  }
}
