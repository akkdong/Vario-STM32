package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-24.
 */

public class SensorData extends AbstractData {
    public int mMask; // valid data mask
    public double mAccelX;  // 3-axis accelerometer value
    public double mAccelY;
    public double mAccelZ;
    public double mGyroX;   // 3-axis gyroscope value
    public double mGyroY;
    public double mGyroZ;
    public double mPressure; // barometer value

    public static final int VALID_ACCEL = 0x0001;
    public static final int VALID_GYRO = 0x0002;
    public static final int VALID_PRESSURE = 0x0003;

    public SensorData() {
        super(AbstractData.DATA_SENSOR);

        mMask = 0;
        mAccelX = mAccelY = mAccelZ = 0;
        mGyroX = mGyroY = mGyroZ = 0;
        mPressure = 0;
    }
}
