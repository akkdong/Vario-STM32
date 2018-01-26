package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-24.
 */

public class SensorParser {

    public static SensorData parse(String message) {
        SensorData data = new SensorData();

        // $SENSOR,<ax>,<ay>,<az>,<gx>,<gy>,<gz>,<p>,<1>,*checksum
        String[] tokens = message.split(",");

        if (tokens.length == 10) {
            data.mMask = 0; // valid nothing
            if (! tokens[1].isEmpty()) {
                data.mAccelX = Double.parseDouble(tokens[1]); // #1 : ax
                data.mAccelY = Double.parseDouble(tokens[2]); // #2 : ay
                data.mAccelZ = Double.parseDouble(tokens[3]); // #3 : az
                data.mMask |= SensorData.VALID_ACCEL;
            }
            if (! tokens[4].isEmpty()) {
                data.mGyroX = Double.parseDouble(tokens[4]); // #4 : gx
                data.mGyroY = Double.parseDouble(tokens[5]); // #5 : gy
                data.mGyroZ = Double.parseDouble(tokens[6]); // #6 : gz
                data.mMask |= SensorData.VALID_GYRO;
            }
            if (! tokens[7].isEmpty()) {
                data.mPressure = Double.parseDouble(tokens[7]); // #7 : p
                data.mMask |= SensorData.VALID_PRESSURE;
            }
        }

        return data;
    }
}
