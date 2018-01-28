package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-24.
 */

public class LocationParser {
    public LocationData locData = new LocationData();

    // sentence time
    // parse flag : 0(invalid) 1(parse only GPRMC), 2(parse only GPGGA), 3(parse all, ready)
    public int mTime = 0;
    public int mFlag = 0;

    //
    public static final int NMEA_GPRMC = 0x0001;
    public static final int NMEA_GPGGA = 0x0002;

    //
    public void parse(String str, int type /* GPGGA or GPRMC */) {
        // prase GPGGA or GPRMC
        // compare mTime with parse time
        //   if same : oring flag with type
        //   if diff : set flag to type
        switch (type) {
            case NMEA_GPRMC :
                parseRMC(str);
                break;
            case NMEA_GPGGA :
                parseGGA(str);
                break;
        }
    }

    private void parseRMC(String str) {
        // $GPRMC,123519.000,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
        //`
        // 123519.sss   Fix taken at 12:35:19 UTC                   1
        // A            Status A=active or V=Void.                  2
        // 4807.038,N   Latitude 48 deg 07.038' N                   3,4
        // 01131.000,E  Longitude 11 deg 31.000' E                  5,6
        // 022.4        Speed over the ground in knots              7
        // 084.4        Track angle in degrees True                 8
        // 230394       Date - 23rd of March 1994                   9
        // 003.1,W      Magnetic Variation                          10,11
        // *6A          The checksum data, always begins with *     12

        String[] tokens = str.split(",");

        if (tokens[2].charAt(0) == 'A') {
            int time = (int)Float.parseFloat(tokens[1]); // Integer.parseInt(tokens[1]);
            if (mTime != time) {
                mTime = time;
                mFlag = NMEA_GPRMC;
            } else {
                mFlag |= NMEA_GPRMC;
            }

            locData.mSpeed = Double.parseDouble(tokens[7]);
            locData.mTrackangle = Double.parseDouble(tokens[8]);
        }
    }

    private void parseGGA(String str) {
        // $GPGGA,123519.000,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
        //
        // 123519.sss       Fix taken at 12:35:19 UTC                       1
        // 4807.038,N       Latitude 48 deg 07.038' N                       2,3
        // 01131.000,E      Longitude 11 deg 31.000' E                      4,5
        // 1                Fix quality: 0 = invalid                        6
        //                              1 = GPS fix (SPS)
        //                              2 = DGPS fix
        //                              3 = PPS fix
        //                              4 = Real Time Kinematic
        //                              5 = Float RTK
        //                              6 = estimated (dead reckoning)
        //                              7 = Manual input mode
        //                              8 = Simulation mode
        // 08               Number of satellites being tracked              7
        // 0.9              Horizontal dilution of position                 8
        // 545.4,M          Altitude, Meters, above mean sea level          9,10
        // 46.9,M           Height of geoid (mean sea level) above WGS84    10,11
        //                           ellipsoid
        // (empty field)    time in seconds since last DGPS update          12
        // (empty field)    DGPS station ID number                          13
        // *47              the checksum data, always begins with *         14

        String[] tokens = str.split(",");

        if (Integer.parseInt(tokens[6]) != 0) {
            int time = (int)Float.parseFloat(tokens[1]); // Integer.parseInt(tokens[1]);
            if (mTime != time) {
                mTime = time;
                mFlag = NMEA_GPGGA;
            } else {
                mFlag |= NMEA_GPGGA;
            }

            locData.mLatitude = Double.parseDouble(tokens[2]);
            locData.mLongitude = Double.parseDouble(tokens[4]);
            locData.mAltitude = Double.parseDouble(tokens[9]);

            if (tokens[3].charAt(0) == 'S')
                locData.mLatitude = -locData.mLatitude;

            if (tokens[5].charAt(0) == 'W')
                locData.mLongitude = -locData.mLongitude;
        }
    }

    public boolean isReady() {
        return mFlag == (NMEA_GPRMC | NMEA_GPGGA);
    }

    public LocationData getData() {
        if (isReady())
            return locData;

        return null;
    }
}
