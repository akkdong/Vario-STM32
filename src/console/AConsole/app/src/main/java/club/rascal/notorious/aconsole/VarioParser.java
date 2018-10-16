package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-24.
 */

public class VarioParser {
    // ..

    public static VarioData parse(String message) {
        VarioData data = new VarioData();

        // $LK8EX1,<pressure>,<altitude>,<vario>,<temperature>,<battery>,*checksum
        String[] tokens = message.split(",");

        if (tokens.length == 7) {
            // #1 : pressure
            data.mPressure = Double.parseDouble(tokens[1]) / 100;
            // #2 : altitude
            data.mAltitude = Double.parseDouble(tokens[2]);
            // #3 : vario
            data.mVario = Double.parseDouble(tokens[3]);
            // #4 : temperature
            data.mTemperature = Double.parseDouble(tokens[4]);
            // #5 : battery
            data.mBattery = Double.parseDouble(tokens[5]);
        }

        return data;
    }
}
