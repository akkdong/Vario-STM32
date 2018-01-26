package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-25.
 */

public class VarioParam {
    // GliderInfo
    public static final int GLIDER_TYPE 					= 0x1001;
    public static final int GLIDER_MANUFACTURE 			= 0x1002;
    public static final int GLIDER_MODEL 					= 0x1003;

    // IGCLogger
    public static final int LOGGER_ENABLE 				    = 0x1101;
    public static final int LOGGER_TAKEOFF_SPEED 			= 0x1102;
    public static final int LOGGER_LANDING_TIMEOUT 		= 0x1103;
    public static final int LOGGER_LOGGING_INTERVAL 		= 0x1104;
    public static final int LOGGER_PILOT 					= 0x1105;
    public static final int LOGGER_TIMEZONE 				= 0x1106;

    // VarioSettings
    public static final int VARIO_SINK_THRESHOLD			= 0x1201;
    public static final int VARIO_CLIMB_THRESHOLD			= 0x1202;
    public static final int VARIO_SENSITIVITY				= 0x1203;
    public static final int VARIO_SENTENCE				    = 0x1204;
    public static final int VARIO_BAROONLY				    = 0x1205;

    // ToneTables
    public static final int TONETABLE_00_VELOCITY			= 0x1301;
    public static final int TONETABLE_00_FREQ				= 0x1302;
    public static final int TONETABLE_00_PERIOD			= 0x1303;
    public static final int TONETABLE_00_DUTY				= 0x1304;
    public static final int TONETABLE_01_VELOCITY			= 0x1311;
    public static final int TONETABLE_01_FREQ				= 0x1312;
    public static final int TONETABLE_01_PERIOD			= 0x1313;
    public static final int TONETABLE_01_DUTY				= 0x1314;
    public static final int TONETABLE_02_VELOCITY			= 0x1321;
    public static final int TONETABLE_02_FREQ				= 0x1322;
    public static final int TONETABLE_02_PERIOD			= 0x1323;
    public static final int TONETABLE_02_DUTY				= 0x1324;
    public static final int TONETABLE_03_VELOCITY			= 0x1331;
    public static final int TONETABLE_03_FREQ				= 0x1332;
    public static final int TONETABLE_03_PERIOD			= 0x1333;
    public static final int TONETABLE_03_DUTY				= 0x1334;
    public static final int TONETABLE_04_VELOCITY			= 0x1341;
    public static final int TONETABLE_04_FREQ				= 0x1342;
    public static final int TONETABLE_04_PERIOD			= 0x1343;
    public static final int TONETABLE_04_DUTY				= 0x1344;
    public static final int TONETABLE_05_VELOCITY			= 0x1351;
    public static final int TONETABLE_05_FREQ				= 0x1352;
    public static final int TONETABLE_05_PERIOD			= 0x1353;
    public static final int TONETABLE_05_DUTY				= 0x1354;
    public static final int TONETABLE_06_VELOCITY			= 0x1361;
    public static final int TONETABLE_06_FREQ				= 0x1362;
    public static final int TONETABLE_06_PERIOD			= 0x1363;
    public static final int TONETABLE_06_DUTY				= 0x1364;
    public static final int TONETABLE_07_VELOCITY			= 0x1371;
    public static final int TONETABLE_07_FREQ				= 0x1372;
    public static final int TONETABLE_07_PERIOD			= 0x1373;
    public static final int TONETABLE_07_DUTY				= 0x1374;
    public static final int TONETABLE_08_VELOCITY			= 0x1381;
    public static final int TONETABLE_08_FREQ				= 0x1382;
    public static final int TONETABLE_08_PERIOD			= 0x1383;
    public static final int TONETABLE_08_DUTY				= 0x1384;
    public static final int TONETABLE_09_VELOCITY			= 0x1391;
    public static final int TONETABLE_09_FREQ				= 0x1392;
    public static final int TONETABLE_09_PERIOD			= 0x1393;
    public static final int TONETABLE_09_DUTY				= 0x1394;
    public static final int TONETABLE_10_VELOCITY			= 0x13A1;
    public static final int TONETABLE_10_FREQ				= 0x13A2;
    public static final int TONETABLE_10_PERIOD			= 0x13A3;
    public static final int TONETABLE_10_DUTY				= 0x13A4;
    public static final int TONETABLE_11_VELOCITY			= 0x13B1;
    public static final int TONETABLE_11_FREQ				= 0x13B2;
    public static final int TONETABLE_11_PERIOD			= 0x13B3;
    public static final int TONETABLE_11_DUTY				= 0x13B4;

    // VolumeSettings
    public static final int VOLUME_VARIO					= 0x1401;
    public static final int VOLUME_EFFECT					= 0x1402;

    // ThresholdSettings
    public static final int THRESHOLD_LOW_BATTERY			= 0x1501;
    public static final int THRESHOLD_SHUTDOWN_HOLDTIME	= 0x1502;
    public static final int THRESHOLD_AUTO_SHUTDOWN_VARIO = 0x1503;
    public static final int THRESHOLD_AUTO_SHUTDOWN_UMS	= 0x1504;

    // KalmanParameters
    public static final int KALMAN_VAR_ZMEAS				= 0x1601;
    public static final int KALMAN_VAR_ZACCEL				= 0x1602;
    public static final int KALMAN_VAR_ACCELBIAS			= 0x1603;
    public static final int KALMAN_SIGMA_P				    = 0x1611;
    public static final int KALMAN_SIGMA_A				    = 0x1612;

    // CalibrationData
    public static final int CALDATA_ACCEL_00				= 0x1701;
    public static final int CALDATA_ACCEL_01				= 0x1702;
    public static final int CALDATA_ACCEL_02				= 0x1703;
    public static final int CALDATA_GYRO_00				= 0x1711;
    public static final int CALDATA_GYRO_01				= 0x1712;
    public static final int CALDATA_GYRO_02				= 0x1713;
    public static final int CALDATA_MAG_00				    = 0x1721;
    public static final int CALDATA_MAG_01				    = 0x1722;
    public static final int CALDATA_MAG_02				    = 0x1723;

    //
    public static final int EOF							    = 0xFFFF;
}
