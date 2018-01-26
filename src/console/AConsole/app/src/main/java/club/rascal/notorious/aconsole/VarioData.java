package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-24.
 */

public class VarioData extends AbstractData {
    public double mPressure;        // raw pressure in hPa
    public double mAltitude;        // altitude in meters, relative to QNH (1013.25)
    public double mVario;           // vertical velocity (cm/s)
    public double mTemperature;     // temperature in celsius
    public double mBattery;         // battery voltage or charge percentage

    public VarioData() {
        super(AbstractData.DATA_VARIO);

        mPressure = 0;
        mAltitude = 0;
        mVario = 0;
        mTemperature = 0;
        mBattery = 0;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder(256);

        //
        sb.append('$');
        sb.append("VARIO");
        // pressure
        sb.append(',');
        sb.append(mPressure);
        // altitude
        sb.append(',');
        sb.append(mAltitude);
        // vario
        sb.append(',');
        sb.append(mVario);
        // temperature
        sb.append(',');
        sb.append(mTemperature);
        // battery
        sb.append(',');
        sb.append(mBattery);

        return sb.toString();
    }
}