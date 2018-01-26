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
}