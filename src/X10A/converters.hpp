#ifndef CONVERTERS_H
#define CONVERTERS_H

#include "ArduinoC.hpp"
#include "Config/parameterDef.hpp"
#include "MQTT/mqttSerial.hpp"

namespace ESPAltherma
{
    class Converter
    {
    public:
        void convert(ParameterDef *def, char *data);

    private:
        void convertTable300(char *data, int tableID, char *ret);

        void convertTable203(char *data, char *ret);

        void convertTable204(char *data, char *ret);

        void convertTable315(char *data, char *ret);

        void convertTable316(char *data, char *ret);

        void convertTable200(char *data, char *ret);

        void convertTable217(char *data, char *ret);

        unsigned short getUnsignedValue(char *data, int dataSize, int cnvflg);

        short getSignedValue(char *data, int datasize, int cnvflg);
    };

}

extern ESPAltherma::Converter converter;

#endif