#include "ELM327.hpp"

bool DriverELM327::initInterface(CANPort* port, int baudrate)
{
    canPort = port;

    canPort->write("AT Z\r");  // just reset ELM327
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    canPort->write("AT E0\r"); // disable echo
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    uint8_t dividor = 500 / baudrate;
    char baudrateCmd[15];
    sprintf(baudrateCmd, "AT PP 2F SV %02x\r", dividor);

    canPort->write(baudrateCmd); // set given CAN-Bus baudrate
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    canPort->write("AT PP 2F ON\r"); // Activate/save baud parameter
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    canPort->write("AT SP C\r");
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    return true;
}

const char *DriverELM327::sendCommandWithID(CommandDef* cmd, bool setValue)
{
    if(setValue)
    {
        canPort->write("ATSH680\r");
    }
    else
    {
        char message[9];
        sprintf (message, "ATSH%d\r", cmd->id);
        canPort->write(message);
    }

    if(canPort->read() != "OK");
    {
        // error
        return nullptr;
    }

    if(cmd->writable)
    {
        byte modifiedCommand[COMMAND_BYTE_LENGTH];

    }

    /*
     if setValue and type:
            command = cmd[:1] + '2' + cmd[2:]
            if command[6:8] != "FA":
                command = command[:3]+"00 FA"+command[2:8]
            command = command[:14]

            if type == "int":
                setValue = int(setValue)
                command = command+" %02X 00" % (setValue)
            if type == "longint":
                setValue = int(setValue)
                command = command+" 00 %02X" % (setValue)
            if type == "float":
                setValue = int(setValue)
                if setValue < 0:
                    setValue = 0x10000+setValue
                command = command+" %02X %02X" % (setValue >> 8, setValue & 0xff)
            if type == "value":
                setValue = int(setValue)
                command = command+" 00 %02X" % (setValue)

            #self.hpsu.printd("info", "cmd: %s cmdMod: %s" % (cmd, command))
            cmd = command

        self.ser.write(bytes(str("%s\r\n" % cmd).encode('utf-8')))
        time.sleep(50.0 / 1000.0)
        if setValue and type:
            return "OK"

        ser_read = self.ser.read(size=100)
        rc = ser_read.decode('utf-8')[:-3]
        rc = rc.replace("\r", "").replace("\n", "").strip()
        return rc

        ---> rc

        if setValue:
            return rc

        if rc[0:1] != cmd["command"][0:1]:
            self.resetInterface()
            self.hpsu.printd('warning', 'sending cmd %s (rc:%s)' % (cmd["command"], rc))
            return "KO"

    */

    return nullptr;
}