package club.rascal.notorious.aconsole;

/**
 * Created by Jasmine on 2018-01-24.
 */

public class LocationData extends AbstractData {
    public double mLatitude;
    public double mLongitude;
    public double mAltitude;
    public double mSpeed;
    public double mTrackangle;

    public LocationData() {
        super(AbstractData.DATA_LOCATION);

        mLatitude = 0;
        mLongitude = 0;
        mAltitude = 0;
        mSpeed = 0;
        mTrackangle = 0;
    }

    public String toString() {
        StringBuilder sb = new StringBuilder(256);

        //
        sb.append('$');
        sb.append("LOCATION");
        // latitude
        sb.append(',');
        sb.append(mLatitude);
        // longitude
        sb.append(',');
        sb.append(mLongitude);
        // altitude
        sb.append(',');
        sb.append(mAltitude);
        // speed
        sb.append(',');
        sb.append(mSpeed);
        // track-angle
        sb.append(',');
        sb.append(mTrackangle);

        return sb.toString();
    }
}