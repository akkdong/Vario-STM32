package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-25.
 */

public class VarioParam {
    // GliderInfo
    public static final int GLIDER_TYPE 					= 0x9001;
    public static final int GLIDER_MANUFACTURE 			= 0x9002;
    public static final int GLIDER_MODEL 					= 0x9003;

    // IGCLogger
    public static final int LOGGER_ENABLE 				    = 0x9101;
    public static final int LOGGER_TAKEOFF_SPEED 			= 0x9102;
    public static final int LOGGER_LANDING_TIMEOUT 		= 0x9103;
    public static final int LOGGER_LOGGING_INTERVAL 		= 0x9104;
    public static final int LOGGER_PILOT 					= 0x9105;
    public static final int LOGGER_TIMEZONE 				= 0x9106;

    // VarioSettings
    public static final int VARIO_SINK_THRESHOLD			= 0x9201;
    public static final int VARIO_CLIMB_THRESHOLD			= 0x9202;
    public static final int VARIO_SENSITIVITY				= 0x9203;
    public static final int VARIO_SENTENCE				    = 0x9204;
    public static final int VARIO_BAROONLY				    = 0x9205;
    public static final int VARIO_DAMPING_FACTOR         = 0x9206;

    // ToneTables
    public static final int TONETABLE_00_VELOCITY			= 0x9301;
    public static final int TONETABLE_00_FREQ				= 0x9302;
    public static final int TONETABLE_00_PERIOD			= 0x9303;
    public static final int TONETABLE_00_DUTY				= 0x9304;
    public static final int TONETABLE_01_VELOCITY			= 0x9311;
    public static final int TONETABLE_01_FREQ				= 0x9312;
    public static final int TONETABLE_01_PERIOD			= 0x9313;
    public static final int TONETABLE_01_DUTY				= 0x9314;
    public static final int TONETABLE_02_VELOCITY			= 0x9321;
    public static final int TONETABLE_02_FREQ				= 0x9322;
    public static final int TONETABLE_02_PERIOD			= 0x9323;
    public static final int TONETABLE_02_DUTY				= 0x9324;
    public static final int TONETABLE_03_VELOCITY			= 0x9331;
    public static final int TONETABLE_03_FREQ				= 0x9332;
    public static final int TONETABLE_03_PERIOD			= 0x9333;
    public static final int TONETABLE_03_DUTY				= 0x9334;
    public static final int TONETABLE_04_VELOCITY			= 0x9341;
    public static final int TONETABLE_04_FREQ				= 0x9342;
    public static final int TONETABLE_04_PERIOD			= 0x9343;
    public static final int TONETABLE_04_DUTY				= 0x9344;
    public static final int TONETABLE_05_VELOCITY			= 0x9351;
    public static final int TONETABLE_05_FREQ				= 0x9352;
    public static final int TONETABLE_05_PERIOD			= 0x9353;
    public static final int TONETABLE_05_DUTY				= 0x9354;
    public static final int TONETABLE_06_VELOCITY			= 0x9361;
    public static final int TONETABLE_06_FREQ				= 0x9362;
    public static final int TONETABLE_06_PERIOD			= 0x9363;
    public static final int TONETABLE_06_DUTY				= 0x9364;
    public static final int TONETABLE_07_VELOCITY			= 0x9371;
    public static final int TONETABLE_07_FREQ				= 0x9372;
    public static final int TONETABLE_07_PERIOD			= 0x9373;
    public static final int TONETABLE_07_DUTY				= 0x9374;
    public static final int TONETABLE_08_VELOCITY			= 0x9381;
    public static final int TONETABLE_08_FREQ				= 0x9382;
    public static final int TONETABLE_08_PERIOD			= 0x9383;
    public static final int TONETABLE_08_DUTY				= 0x91384;
    public static final int TONETABLE_09_VELOCITY			= 0x9391;
    public static final int TONETABLE_09_FREQ				= 0x9392;
    public static final int TONETABLE_09_PERIOD			= 0x9393;
    public static final int TONETABLE_09_DUTY				= 0x9394;
    public static final int TONETABLE_10_VELOCITY			= 0x93A1;
    public static final int TONETABLE_10_FREQ				= 0x93A2;
    public static final int TONETABLE_10_PERIOD			= 0x93A3;
    public static final int TONETABLE_10_DUTY				= 0x93A4;
    public static final int TONETABLE_11_VELOCITY			= 0x93B1;
    public static final int TONETABLE_11_FREQ				= 0x93B2;
    public static final int TONETABLE_11_PERIOD			= 0x93B3;
    public static final int TONETABLE_11_DUTY				= 0x93B4;

    // VolumeSettings
    public static final int VOLUME_VARIO					= 0x9401;
    public static final int VOLUME_EFFECT					= 0x9402;

    // ThresholdSettings
    public static final int THRESHOLD_LOW_BATTERY			= 0x9501;
    public static final int THRESHOLD_SHUTDOWN_HOLDTIME	= 0x9502;
    public static final int THRESHOLD_AUTO_SHUTDOWN_VARIO = 0x9503;
    public static final int THRESHOLD_AUTO_SHUTDOWN_UMS	= 0x9504;

    // KalmanParameters
    public static final int KALMAN_VAR_ZMEAS				= 0x9601;
    public static final int KALMAN_VAR_ZACCEL				= 0x9602;
    public static final int KALMAN_VAR_ACCELBIAS			= 0x9603;
    public static final int KALMAN_SIGMA_P				    = 0x9611;
    public static final int KALMAN_SIGMA_A				    = 0x9612;

    // CalibrationData
    public static final int CALDATA_ACCEL_00				= 0x9701;
    public static final int CALDATA_ACCEL_01				= 0x9702;
    public static final int CALDATA_ACCEL_02				= 0x9703;
    public static final int CALDATA_GYRO_00				= 0x9711;
    public static final int CALDATA_GYRO_01				= 0x9712;
    public static final int CALDATA_GYRO_02				= 0x9713;
    public static final int CALDATA_MAG_00				    = 0x9721;
    public static final int CALDATA_MAG_01				    = 0x9722;
    public static final int CALDATA_MAG_02				    = 0x9723;

    //
    public static final int EOF							    = 0xFFFF;
}
