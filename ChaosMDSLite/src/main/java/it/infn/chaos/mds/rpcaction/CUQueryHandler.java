/**
 *
 */
package it.infn.chaos.mds.rpcaction;

import it.infn.chaos.mds.RPCConstants;
import it.infn.chaos.mds.SingletonServices;
import it.infn.chaos.mds.batchexecution.SystemCommandWorkUnit;
import it.infn.chaos.mds.batchexecution.UnitServerACK;
import it.infn.chaos.mds.batchexecution.WorkUnitACK;
import it.infn.chaos.mds.batchexecution.SystemCommandWorkUnit.LoadUnloadWorkUnitSetting;
import it.infn.chaos.mds.business.Device;
import it.infn.chaos.mds.business.DeviceClass;
import it.infn.chaos.mds.business.UnitServer;
import it.infn.chaos.mds.business.UnitServerCuInstance;
import it.infn.chaos.mds.da.DeviceDA;
import it.infn.chaos.mds.da.UnitServerDA;
import it.infn.chaos.mds.event.ChaosEventComponent;
import it.infn.chaos.mds.event.EventsToVaadin;
import it.infn.chaos.mds.rpc.server.RPCActionHadler;

import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.sql.SQLException;
import java.util.HashSet;
import java.util.ListIterator;
import java.util.Set;
import java.util.Vector;

import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.ref.common.exception.RefException;
import org.ref.common.helper.ExceptionHelper;

/**
 * RPC actions for manage the Control Unit registration and device dataset retriving
 * 
 * @author bisegni
 */
public class CUQueryHandler extends RPCActionHadler {
	private static final String	SYSTEM					= "system";
	private static final String	REGISTER_CONTROL_UNIT	= "registerControlUnit";
	private static final String	HEARTBEAT_CONTROL_UNIT	= "heartbeatControlUnit";
	private static final String	UNIT_SERVER_STATES		= "unit_server_states";

	/*
	 * (non-Javadoc)
	 * 
	 * @see it.infn.chaos.mds.rpc.server.RPCActionHadler#intiHanlder()
	 */
	@Override
	public void intiHanlder() throws RefException {
		addDomainAction(SYSTEM, RPCConstants.MDS_REGISTER_UNIT_SERVER);
		addDomainAction(SYSTEM, CUQueryHandler.REGISTER_CONTROL_UNIT);
		addDomainAction(SYSTEM, CUQueryHandler.UNIT_SERVER_STATES);

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see it.infn.chaos.mds.rpc.server.RPCActionHadler#handleAction(java.lang.String , java.lang.String, org.bson.BasicBSONObject)
	 */
	@Override
	public BasicBSONObject handleAction(String domain, String action, BasicBSONObject actionData) throws RefException {
		BasicBSONObject result = null;
		if (domain.equals(SYSTEM)) {
			if (action.equals(REGISTER_CONTROL_UNIT)) {
				result = registerControUnit(actionData);
			} else if (action.equals(HEARTBEAT_CONTROL_UNIT)) {
				result = heartbeat(actionData);
			} else if (action.equals(UNIT_SERVER_STATES)) {
				result = unit_server_states(actionData);

			} else if (action.equals(RPCConstants.MDS_REGISTER_UNIT_SERVER)) {
				try {
					result = registerUnitServer(actionData);
				} catch (NoSuchAlgorithmException e) {
					throw new RefException(ExceptionHelper.getInstance().putExcetpionStackToString(e), 0, "CUQueryHandler::handleAction");
				} catch (NoSuchProviderException e) {
					throw new RefException(ExceptionHelper.getInstance().putExcetpionStackToString(e), 0, "CUQueryHandler::handleAction");
				}
			}
		}
		return result;
	}

	/**
	 * @param actionData
	 * @return
	 */
	private BasicBSONObject unit_server_states(BasicBSONObject actionData) {
		UnitServerDA usDA = null;
		DeviceDA dDA = null;

		UnitServer us = null;
		us = new UnitServer();
		us.setAlias(actionData.getString(RPCConstants.MDS_REGISTER_UNIT_SERVER_ALIAS));
		us.setIp_port(actionData.getString(RPCConstants.CONTROL_UNIT_INSTANCE_NETWORK_ADDRESS));
		try {
			Vector<UnitServerCuInstance> instanceForUnitServer = null;

			usDA = getDataAccessInstance(UnitServerDA.class);
			dDA = getDataAccessInstance(DeviceDA.class);

			usDA.updateUnitServerTSAndIP(us);
			us = usDA.getUnitServerByAlias(us.getAlias());
			instanceForUnitServer = usDA.returnAllUnitServerCUAssociationbyUSAlias(us.getAlias());
			for (UnitServerCuInstance cu : instanceForUnitServer) {
				usDA.setState(cu.getCuId(), "---");

			}
			BasicBSONList bsonAttributesArray = (BasicBSONList) actionData.get(RPCConstants.UNIT_SERVER_CU_STATES);
			if (bsonAttributesArray != null) {
				for (Object obj : bsonAttributesArray) {
					BasicBSONObject bobj = (BasicBSONObject) obj;
					if (bobj.containsField("device_key")) {
						String cu_id = bobj.getString("device_key");
						Integer cu_sm_state = bobj.getInt(cu_id + "_sm_state");
						Integer cu_state = bobj.getInt(cu_id + "_state");
						String str_cu_state = null;
						String str_cu_sm_state = null;
						switch (cu_state) {
						case 0:
							str_cu_state = String.format("CUState: %s", "Deinit");
							break;
						case 1:
							str_cu_state = String.format("CUState: %s", "Init");
							break;
						case 2:
							str_cu_state = String.format("CUState: %s", "Start");
							break;
						case 3:
							str_cu_state = String.format("CUState: %s", "Stop");
							break;

						}

						switch (cu_sm_state) {
						case 0:
							str_cu_sm_state = String.format("SMState: %s", "Unpub");
							break;
						case 1:
							str_cu_sm_state = String.format("SMState: %s", "StartPub");
							break;
						case 2:
							str_cu_sm_state = String.format("SMState: %s", "Publishing");
							break;
						case 3:
							str_cu_sm_state = String.format("SMState: %s", "Published");
							break;
						case 4:
							str_cu_sm_state = String.format("SMState: %s", "StartUnpub");
							break;
						case 5:
							str_cu_sm_state = String.format("SMState: %s", "Unpublishing");
							break;
						case 6:
							str_cu_sm_state = String.format("SMState: %s", "Unpublishing");
							break;

						}
						// String state = String.format("CUState: %d CUSMState: %d", args)
						if (cu_state != null) {
							dDA.performDeviceHB(cu_id);
							usDA.setState(cu_id, String.format("%s %s", str_cu_state, str_cu_sm_state));

						}
					}
				}
				us.setCuInstances(instanceForUnitServer);
			}
			ChaosEventComponent ev = ChaosEventComponent.getInstance();
			ev.usUpdate(us);

			closeDataAccess(usDA, true);
			closeDataAccess(dDA, true);

		} catch (Throwable e) {
			e.printStackTrace();
		}
		return null;
	}

	/**
	 * Register an unit server
	 * 
	 * @param actionData
	 * @return
	 * @throws NoSuchProviderException
	 * @throws NoSuchAlgorithmException
	 * @throws SQLException
	 * @throws IllegalAccessException
	 * @throws InstantiationException
	 */
	private BasicBSONObject registerUnitServer(BasicBSONObject actionData) throws RefException, NoSuchAlgorithmException, NoSuchProviderException {
		UnitServerDA usDA = null;
		String registrationPublicKey = null;
		Vector<UnitServerCuInstance> instanceForUnitServer = null;
		LoadUnloadWorkUnitSetting setting = new LoadUnloadWorkUnitSetting();
		UnitServer us = null;
		try {
			usDA = getDataAccessInstance(UnitServerDA.class);
			if (actionData == null)
				new RefException("No info forwarded for unit server registration", 1, "CUQueryHandler::registerUnitServer");
			us = new UnitServer();
			us.setAlias(actionData.getString(RPCConstants.MDS_REGISTER_UNIT_SERVER_ALIAS));
			us.setIp_port(actionData.getString(RPCConstants.CONTROL_UNIT_INSTANCE_NETWORK_ADDRESS));
			us.setState(-1);
			BasicBSONList bsonAttributesArray = (BasicBSONList) actionData.get(RPCConstants.MDS_REGISTER_UNIT_SERVER_CONTROL_UNIT_ALIAS);
			ListIterator<Object> dsDescIter = bsonAttributesArray.listIterator();
			while (dsDescIter.hasNext()) {
				us.addPublischedCU(dsDescIter.next().toString());
			}

			if (actionData.containsField(RPCConstants.MDS_REGISTER_UNIT_SERVER_KEY)) {
				registrationPublicKey = actionData.getString(RPCConstants.MDS_REGISTER_UNIT_SERVER_KEY);
			}
			// the registration pack contain an rsa key check if it match with the private
			if (!usDA.checkPublicKey(us.getAlias(), registrationPublicKey)) {
				actionData.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 6);
				return null;
			}

			if (usDA.unitServerAlreadyRegistered(us)) {
				usDA.updateUnitServerTSAndIP(us);
			} else {
				usDA.insertNewUnitServer(us);
			}
			for (String cuClass : us.getPublischedCU()) {
				DeviceClass dc = new DeviceClass();
				dc.setDeviceClass(cuClass);
				dc.setDeviceClassAlias(cuClass);
				dc.setDeviceSource("pre-existent");
				try {
					usDA.insertNewDeviceClass(dc);
				} catch (Throwable e) {
					// TODO: handle exception
				}

			}
			// now start all association in auto-load for that server, if are present
			instanceForUnitServer = usDA.loadAllAssociationForUnitServerAliasInAutoload(us.getAlias());
			Set<UnitServerCuInstance> instanceSet = new HashSet<UnitServerCuInstance>();
			instanceSet.addAll(instanceForUnitServer);
			setting.unit_server_container = usDA.getUnitServerByAlias(us.getAlias());
			setting.loadUnload = true;
			setting.associations = instanceSet;
			instanceForUnitServer = usDA.loadAllAssociationForUnitServerAliasInAutoload(us.getAlias());
			us.setState(5);
			usDA.updateUnitServerTSAndIP(us);

			actionData.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 5);
			closeDataAccess(usDA, true);
			try {
				ChaosEventComponent ev = ChaosEventComponent.getInstance();
				ev.unitRegistrationEvent(us.getAlias());
			} catch (Throwable e) {
				e.printStackTrace();
			}
		} catch (InstantiationException e) {
			actionData.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 6);

			throw new RefException(ExceptionHelper.getInstance().putExcetpionStackToString(e), 0, "CUQueryHandler::registerUnitServer");
		} catch (IllegalAccessException e) {
			actionData.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 6);
			throw new RefException(ExceptionHelper.getInstance().putExcetpionStackToString(e), 1, "CUQueryHandler::registerUnitServer");
		} catch (SQLException e) {
			actionData.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 6);
			throw new RefException(ExceptionHelper.getInstance().putExcetpionStackToString(e), 2, "CUQueryHandler::registerUnitServer");
		} finally {
			try {
				closeDataAccess(usDA, false);
			} catch (SQLException e) {
				// e.printStackTrace();
			}
			try {
				SingletonServices.getInstance().getSlowExecution().submitJob(UnitServerACK.class.getName(), actionData);
				// check if we have association in auto-load
				if (setting.unit_server_container != null && setting.associations != null && setting.associations.size() > 0) {
					SingletonServices.getInstance().getSlowExecution().submitJob(SystemCommandWorkUnit.class.getName(), setting, 2);
				}
			} catch (InstantiationException e) {
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
			}
		}

		return null;
	}

	/**
	 * Execute l'heartbeat of the control unit
	 * 
	 * @param actionData
	 * @return
	 */
	private BasicBSONObject heartbeat(BasicBSONObject actionData) throws RefException {
		BasicBSONObject result = null;
		if (!actionData.containsField(RPCConstants.DATASET_DEVICE_ID))
			throw new RefException("Message desn't contain deviceid");
		DeviceDA dDA = null;
		try {
			dDA = getDataAccessInstance(DeviceDA.class);
			dDA.performDeviceHB(actionData.getString(RPCConstants.DATASET_DEVICE_ID));
		} catch (Exception e) {
			throw new RefException("heartbeat error", 1, "CUQueryHandler::heartbeat");
		} catch (Throwable e) {
			throw new RefException("heartbeat error", 1, "CUQueryHandler::heartbeat");
		} finally {
			try {
				closeDataAccess(dDA, false);
			} catch (SQLException e) {
				e.printStackTrace();
			}
		}

		return result;
	}

	/**
	 * @return
	 * @throws Throwable
	 */
	private BasicBSONObject registerControUnit(BasicBSONObject actionData) throws RefException {
		Device d = null;
		BasicBSONObject result = null;
		DeviceDA dDA = null;
		UnitServerDA usDA = null;
		BasicBSONObject ackPack = new BasicBSONObject();
		boolean sendACK = false;
		try {
			dDA = getDataAccessInstance(DeviceDA.class);
			usDA = getDataAccessInstance(UnitServerDA.class);

			if (actionData == null)
				return null;
			// check for CU presence
			String controlUnitNetAddress = actionData.containsField(RPCConstants.CONTROL_UNIT_INSTANCE_NETWORK_ADDRESS) ? actionData.getString(RPCConstants.CONTROL_UNIT_INSTANCE_NETWORK_ADDRESS) : null;
			if (controlUnitNetAddress == null)
				throw new RefException("No network address found", 0, "DeviceDA::controlUnitValidationAndRegistration");
			String controlUnitInstance = actionData.containsField(RPCConstants.CONTROL_UNIT_INSTANCE) ? actionData.getString(RPCConstants.CONTROL_UNIT_INSTANCE) : null;
			if (controlUnitInstance == null)
				throw new RefException("No control unit instance found", 1, "DeviceDA::controlUnitValidationAndRegistration");
			if (!actionData.containsField("mds_control_key")) {
				throw new RefException("No self_managed_work_unit key found", 1, "DeviceDA::controlUnitValidationAndRegistration");
			}

			String control_key = actionData.getString("mds_control_key");

			d = new Device();
			d.setCuInstance(controlUnitInstance);
			d.setNetAddress(controlUnitNetAddress);
			d.fillFromBson(actionData);

			// add device id into ack pack
			ackPack.append(RPCConstants.CONTROL_UNIT_INSTANCE_NETWORK_ADDRESS, actionData.getString(RPCConstants.CONTROL_UNIT_INSTANCE_NETWORK_ADDRESS));
			ackPack.append(RPCConstants.DATASET_DEVICE_ID, actionData.getString(RPCConstants.DATASET_DEVICE_ID));
			sendACK = true;
			if (usDA.cuIDIsMDSManaged(actionData.getString(RPCConstants.DATASET_DEVICE_ID)) && !control_key.equals("mds")) {
				ackPack.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 9);
				return null;
			}
			// check for deviceID presence
			if (dDA.isDeviceIDPresent(d.getDeviceIdentification())) {
				// the device is already present i need to check for dataset
				// change

				if (dDA.isDSChanged(d.getDeviceIdentification(), d.getDataset().getAttributes())) {
					Integer deviceID = dDA.getDeviceIdFormInstance(d.getDeviceIdentification());
					d.getDataset().setDeviceID(deviceID);

					// configura dataset wth configured value
					usDA.configuraDataseAttributestForCUID(d.getDeviceIdentification(), d.getDataset().getAttributes());

					// add new dataset
					dDA.insertNewDataset(d.getDataset());
				} else {
					usDA.configuraDataseAttributestForCUID(d.getDeviceIdentification(), d.getDataset().getAttributes());
					dDA.updateDatasetAttributeValue(d.getDataset());
				}
				// update the CU id for this device, it can be changed
				dDA.updateCUInstanceAndAddressForDeviceID(d.getDeviceIdentification(), d.getCuInstance(), d.getNetAddress());
			} else {
				// configure the dataset
				usDA.configuraDataseAttributestForCUID(d.getDeviceIdentification(), d.getDataset().getAttributes());

				dDA.insertDevice(d);
			}

			if (d != null) {
				dDA.performDeviceHB(d.getDeviceIdentification());
				usDA.setState(d.getDeviceIdentification(), "Registered");
			}

			closeDataAccess(dDA, true);
			closeDataAccess(usDA, true);
			ackPack.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 5);
			try {
				ChaosEventComponent ev = ChaosEventComponent.getInstance();
				ev.deviceRegistrationEvent(controlUnitInstance);
			} catch (Throwable e) {
				e.printStackTrace();
			}
		} catch (RefException e) {
			actionData.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 6);
			try {
				closeDataAccess(dDA, false);
			} catch (SQLException e1) {
			}
			try {
				closeDataAccess(usDA, false);
			} catch (SQLException e1) {
			}
			throw new RefException(e.getMessage(), 3, "CUQueryHandler::registerControUnit");
		} catch (Throwable e) {
			ackPack.append(RPCConstants.MDS_REGISTER_UNIT_SERVER_RESULT, (int) 6);
			try {
				closeDataAccess(dDA, false);
			} catch (SQLException e1) {
			}
			try {
				closeDataAccess(usDA, false);
			} catch (SQLException e1) {
			}
			throw new RefException(e.getMessage(), 4, "CUQueryHandler::registerControUnit");
		} finally {
			try {
				if (sendACK == true)
					SingletonServices.getInstance().getSlowExecution().submitJob(WorkUnitACK.class.getName(), ackPack);
			} catch (InstantiationException e) {
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
			}
		}
		return result;
	}
}
