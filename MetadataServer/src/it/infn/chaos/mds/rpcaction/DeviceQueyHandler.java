/**
 * 
 */
package it.infn.chaos.mds.rpcaction;

import java.util.List;

import it.infn.chaos.mds.RPCConstants;
import it.infn.chaos.mds.business.Device;
import it.infn.chaos.mds.da.DataServerDA;
import it.infn.chaos.mds.da.DeviceDA;
import it.infn.chaos.mds.rpc.server.RPCActionHadler;
import it.infn.chaos.mds.rpc.server.RPCUtils;

import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.ref.common.exception.RefException;

/**
 * RPC actions for manage the Control Unit registration and device dataset retriving
 * 
 * @author bisegni
 */
public class DeviceQueyHandler extends RPCActionHadler {
	private static final String	SYSTEM					= "system";
	private static final String	GET_LAST_DATASET		= "getCurrentDataset";
	private static final String	GET_ALL_ACTIVE_DEVICE	= "getAllActiveDevice";

	/*
	 * (non-Javadoc)
	 * 
	 * @see it.infn.chaos.mds.rpc.server.RPCActionHadler#intiHanlder()
	 */
	@Override
	public void intiHanlder() throws RefException {
		addDomainAction(SYSTEM, GET_LAST_DATASET);
		addDomainAction(SYSTEM, GET_ALL_ACTIVE_DEVICE);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see it.infn.chaos.mds.rpc.server.RPCActionHadler#handleAction(java.lang.String ,
	 * java.lang.String, org.bson.BasicBSONObject)
	 */
	@Override
	public BasicBSONObject handleAction(String domain, String action, BasicBSONObject actionData) throws Throwable {
		BasicBSONObject result = null;
		if (domain.equals(SYSTEM)) {
			if (action.equals(GET_LAST_DATASET)) {
				result = getDeviceDataseFromDeviceID(actionData);
			}else if (action.equals(GET_ALL_ACTIVE_DEVICE)) {
				result = getAllActiveDevice(actionData);
			}
		}
		return result;
	}

	/**
	 * 
	 * @param actionData
	 * @return
	 * @throws Throwable 
	 */
	private BasicBSONObject getAllActiveDevice(BasicBSONObject actionData) throws Throwable {
		DeviceDA dDA = null;
		BasicBSONObject result = new BasicBSONObject();
		try {
			
			dDA = getDataAccessInstance(DeviceDA.class);
			List<Device> devices = dDA.getAllActiveDevice();
			BasicBSONList deviceArray = new BasicBSONList();
			for (Device d : devices) {
				deviceArray.add(d.getDeviceIdentification());
			}
			result.append(RPCConstants.DATASET_DEVICE_ID, deviceArray);
		} catch (Throwable e) {
			RPCUtils.addRefExceptionElementToBson(result, "DeviceQueyHandler::getDeviceDataseFromDeviceID", e.getMessage(), 0);
			throw e;
		} finally {
			closeDataAccess(dDA, false);
		}
		return result;
	}

	/**
	 * Return the last dataset for an device id
	 * 
	 * @param actionData
	 * @return
	 * @throws Throwable
	 */
	private BasicBSONObject getDeviceDataseFromDeviceID(BasicBSONObject actionData) throws Throwable {
		DeviceDA dDA = null;
		DataServerDA dsDA = null;
		BasicBSONObject result = null;
		try {
			dDA = getDataAccessInstance(DeviceDA.class);
			dsDA = getDataAccessInstance(DataServerDA.class);
			String deviceIdentification = actionData.getString(RPCConstants.DATASET_DEVICE_ID);
			result = DeviceDescriptionUtility.composeStartupCommandForDeviceIdentification(deviceIdentification, dDA, dsDA);
		} catch (Throwable e) {
			RPCUtils.addRefExceptionElementToBson(result, "DeviceQueyHandler::getDeviceDataseFromDeviceID", e.getMessage(), 0);
			throw e;
		} finally {
			if (dDA != null)
				dDA.getConnection().close();
			if (dsDA != null)
				dsDA.getConnection().close();
		}
		return result;
	}
}
