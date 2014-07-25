/**
 * 
 */
package it.infn.chaos.mds.business;

import java.util.Vector;

import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.ref.common.business.BusinessObject;
import org.ref.common.business.DBColumn;
import org.ref.common.business.DBTable;

import com.mongodb.util.JSON;
import com.mongodb.util.JSONSerializers;

/**
 * @author bisegni
 * 
 */
@DBTable(name = "unit_server_cu_instance")
public class UnitServerCuInstance extends BusinessObject {
	private static final long	serialVersionUID	= -3540643481390952382L;
	public static final String	UNIT_SERVER_ALIAS	= "unit_server_alias";
	public static final String	CU_ID				= "cu_id";
	public static final String	CU_TYPE				= "cu_type";
	public static final String	CU_PARAM			= "cu_param";
	public static final String	DRIVER_SPEC			= "driver_spec";
	public static final String	AUTO_LOAD			= "auto_load";

	@DBColumn(name = UNIT_SERVER_ALIAS, maxDimension = 64)
	private String				unitServerAlias		= null;
	@DBColumn(name = CU_ID, maxDimension = 64)
	private String				cuId				= null;
	@DBColumn(name = CU_TYPE, maxDimension = 64)
	private String				cuType				= null;
	@DBColumn(name = DRIVER_SPEC)
	private String				drvSpec				= null;
	private Vector<DriverSpec>	driverSpec			= new Vector<DriverSpec>();

	public String getCuType() {
		return cuType;
	}

	public void setCuType(String cuType) {
		this.cuType = cuType;
	}

	@DBColumn(name = CU_PARAM, maxDimension = 64)
	private String	cuParam		= null;
	@DBColumn(name = AUTO_LOAD)
	private Boolean	autoLoad	= false;

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ref.common.business.BusinessObject#equals(java.lang.Object)
	 */
	@Override
	public boolean equals(Object arg0) {
		// TODO Auto-generated method stub
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ref.common.business.BusinessObject#hashCode()
	 */
	@Override
	public int hashCode() {
		// TODO Auto-generated method stub
		return 0;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ref.common.business.BusinessObject#toString()
	 */
	@Override
	public String toString() {
		// TODO Auto-generated method stub
		return null;
	}

	public String getUnitServerAlias() {
		return unitServerAlias;
	}

	public void setUnitServerAlias(String unitServerAlias) {
		this.unitServerAlias = unitServerAlias;
	}

	public String getCuId() {
		return cuId;
	}

	public void setCuId(String cuId) {
		this.cuId = cuId;
	}

	public String getCuParam() {
		return cuParam;
	}

	public void setCuParam(String cuParam) {
		this.cuParam = cuParam;
	}

	public Boolean getAutoLoad() {
		return autoLoad;
	}

	public void setAutoLoad(Boolean autoLoad) {
		this.autoLoad = autoLoad;
	}

	public Vector<DriverSpec> getDriverSpec() {
		return driverSpec;
	}

	public void setDriverSpec(Vector<DriverSpec> driverSpec) {
		this.driverSpec = driverSpec;
	}

	public String getDriverSpecInString() {
		StringBuffer sb = new StringBuffer();
		for (DriverSpec ds : getDriverSpec()) {
			sb.append(ds.getDriverName());
			sb.append("||");
			sb.append(ds.getDriverVersion());
			sb.append("||");
			sb.append(ds.getDriverInit());
			sb.append("||");
		}
		if (sb.length() > 0) {
			sb.setLength(sb.length() - 2);
		}
		return sb.toString();
	}

	public BasicBSONObject getDriverSpecAsBson() {
		BasicBSONObject bobj = new BasicBSONObject();
		bobj.append("US_ALIAS", getUnitServerAlias());
		bobj.append("CU_TYPE", getCuType());
		bobj.append("CU_ID", getCuId());
		if (getDriverSpec().size() > 0) {
			BasicBSONList blist = new BasicBSONList();
			for (DriverSpec ds : getDriverSpec()) {
				BasicBSONObject dspec = new BasicBSONObject();
				dspec.append("DRV_NAME", ds.getDriverName());
				dspec.append("DRV_VERSION", ds.getDriverVersion());
				dspec.append("DRV_INIT", ds.getDriverInit());
				blist.add(dspec);
			}
			bobj.append("CU_DRV_SPEC", blist);
		}
		return bobj;
	}

	public void addDrvierSpec(DriverSpec driverSpec) {
		this.driverSpec.addElement(driverSpec);
	}

	public void clear() {
		this.driverSpec.clear();
	}

	public String getDrvSpec() {
		return drvSpec;
	}

	public void setDrvSpec(String drvSpec) {
		this.drvSpec = drvSpec;
	}
}
