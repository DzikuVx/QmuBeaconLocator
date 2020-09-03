#include "radio_node.h"
#include "lora.h"


uint32_t getFrequencyForChannel(uint8_t channel) {
    return RADIO_FREQUENCY_MIN + (RADIO_CHANNEL_WIDTH * channel);
}

uint8_t getNextChannel(uint8_t channel) {
    return (channel + RADIO_HOP_OFFSET) % RADIO_CHANNEL_COUNT;
}

uint8_t getPrevChannel(uint8_t channel) {
    return (RADIO_CHANNEL_COUNT + channel - RADIO_HOP_OFFSET) % RADIO_CHANNEL_COUNT;
}

RadioNode::RadioNode(void) {

}

void RadioNode::reset(void) {
    set(
        loraTxPower, 
        loraBandwidth, 
        loraSpreadingFactor, 
        loraCodingRate, 
        getFrequencyForChannel(getChannel())
    );
}

void RadioNode::init(uint8_t ss, uint8_t rst, uint8_t di0, void(*callback)(int)) {
    /*
     * Setup hardware
     */
    LoRa.setPins(
        ss,
        rst,
        di0
    );

    if (!LoRa.begin(getFrequencyForChannel(getChannel()))) {
        while (true);
    }
        
    reset();
    LoRa.enableCrc();

    //Setup ISR callback and start receiving
    if (callback != NULL) {
        LoRa.onReceive(callback);
    }
    LoRa.receive();
    radioState = RADIO_STATE_RX;
}

void RadioNode::readRssi(void)
{
    rssi = 164 - constrain(LoRa.packetRssi() * -1, 0, 164);
}

void RadioNode::readSnr(void)
{
    snr = (uint8_t) constrain(LoRa.packetSnr(), 0, 255);
}

uint8_t RadioNode::getChannel(void) {
    return _channel;
}

uint32_t RadioNode::getChannelEntryMillis(void) {
    return _channelEntryMillis;
}

void RadioNode::flush() {
    LoRa.sleep();
    LoRa.receive();
    radioState = RADIO_STATE_RX;
    bytesToRead = NO_DATA_TO_READ;
}

void RadioNode::readAndDecode(
    QspConfiguration_t *qsp
) {

    // if more data than length of the buffer, flush it
    if (bytesToRead > MAX_PACKET_SIZE) {
        flush();
        return;
    }

    uint8_t tmpBuffer[MAX_PACKET_SIZE];

    /*
     * There is data to be read from radio!
     */
    if (bytesToRead != NO_DATA_TO_READ) {
        LoRa.read(tmpBuffer, bytesToRead);

        for (int i = 0; i < bytesToRead; i++) {
            qspDecodingStatus_e status = qspDecodeIncomingFrame(qsp, tmpBuffer[i], bindKey);

            if (status != QSP_DECODING_STATUS_OK) {
                Serial.println("Flushing - damaged frameID");
                flush();
                return;
            }

        }

        //After reading, flush radio buffer, we have no need for whatever might be over there
        LoRa.sleep();
        LoRa.receive();

        radioState = RADIO_STATE_RX;
        bytesToRead = NO_DATA_TO_READ;
    }
}

void RadioNode::hopFrequency(bool forward, uint8_t fromChannel, uint32_t timestamp) {
    _channelEntryMillis = timestamp;

    if (forward) {
        _channel = getNextChannel(fromChannel);
    } else {
        _channel = getPrevChannel(fromChannel);
    }

    // And set hardware
    LoRa.sleep();
    LoRa.setFrequency(
        getFrequencyForChannel(_channel)
    );
    LoRa.idle();
}

bool RadioNode::handleTxDoneState(bool hop) {
    const uint32_t currentMillis = millis();
    
    if (
        radioState == RADIO_STATE_TX &&
        currentMillis > nextTxCheckMillis &&
        !LoRa.isTransmitting()
    ) {

        /*
         * In case of TX module, hop right now
         */
        if (hop) {
            hopFrequency(true, getChannel(), currentMillis);
        }

        LoRa.receive();
        radioState = RADIO_STATE_RX;
        nextTxCheckMillis = currentMillis + 1; //We check of TX done every 1ms
        return true;
    } else {
        return false;
    }
}

void RadioNode::handleTx(QspConfiguration_t *qsp) {

    if (!canTransmit) {
        return;
    }

    uint8_t size;
    uint8_t tmpBuffer[MAX_PACKET_SIZE];

    LoRa.beginPacket();
    //Prepare packet
    qspEncodeFrame(qsp, tmpBuffer, &size, getChannel(), bindKey);
    //Sent it to radio in one SPI transaction
    LoRa.write(tmpBuffer, size);
    LoRa.endPacketAsync();

    //Set state to be able to detect the moment when TX is done
    radioState = RADIO_STATE_TX;
}

void RadioNode::set(
    uint8_t power, 
    long bandwidth, 
    uint8_t spreadingFactor, 
    uint8_t codingRate,
    long frequency
) {
    LoRa.sleep();
    
    LoRa.setTxPower(power, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setSignalBandwidth(bandwidth);
    LoRa.setCodingRate4(codingRate);
    LoRa.setFrequency(frequency);

    LoRa.idle();
}

void RadioNode::configure(
    uint8_t _power, 
    long _bandwidth,
    uint8_t _spreadingFactor, 
    uint8_t _codingRate
) {
    loraTxPower = _power; 
    loraBandwidth = _bandwidth;
    loraSpreadingFactor = _spreadingFactor; 
    loraCodingRate = _codingRate;
}