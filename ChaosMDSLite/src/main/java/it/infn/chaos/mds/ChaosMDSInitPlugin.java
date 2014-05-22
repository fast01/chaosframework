/**
 * 
 */
package it.infn.chaos.mds;

import it.infn.chaos.mds.rpc.server.JMQRPCClient;
import it.infn.chaos.mds.rpc.server.JMQRPCServer;
import it.infn.chaos.mds.rpcaction.CUQueryHandler;
import it.infn.chaos.mds.rpcaction.DeviceQueyHandler;
import it.infn.chaos.mds.rpcaction.PerformanceTest;

import java.util.Arrays;
import java.util.StringTokenizer;
import java.util.Vector;

import org.ref.server.configuration.REFServerConfiguration;
import org.ref.server.plugins.REFDeinitPlugin;
import org.ref.server.plugins.REFInitPlugin;

import com.mongodb.DB;
import com.mongodb.MongoClient;
import com.mongodb.MongoCredential;
import com.mongodb.ServerAddress;
import com.mongodb.WriteConcern;

/**
 * Inizializzaiozne del lato serve di GOVA
 * 
 * @author bisegni
 */
public class ChaosMDSInitPlugin implements REFInitPlugin, REFDeinitPlugin {

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ref.server.slfservletadaptor.RTTFInitPlugin#custoInit()
	 */
	public void customInit() {
		try {
			String port = REFServerConfiguration.getInstance().getStringParamByValue("mds.port");
			String mongodbURL = REFServerConfiguration.getInstance().getStringParamByValue("mongodb.url");
			String user = REFServerConfiguration.getInstance().getStringParamByValue("mongodb.user");
			String pwd = REFServerConfiguration.getInstance().getStringParamByValue("mongodb.pwd");
			String db = REFServerConfiguration.getInstance().getStringParamByValue("mongodb.db");
			if(mongodbURL != null) {
				StringTokenizer tokenizer = new StringTokenizer(mongodbURL, ",");
				Vector<ServerAddress> servers = new Vector<ServerAddress>();
				while(tokenizer.hasMoreTokens()) {
					String databaseUrl = tokenizer.nextToken();
					servers.add(new ServerAddress(databaseUrl));
				}
				if(servers.size()>0) {
					SingletonServices.getInstance().setMongoClient(new MongoClient(servers, Arrays.asList(MongoCredential.createMongoCRCredential(user, db, pwd.toCharArray()))));
					SingletonServices.getInstance().setMongoDB( SingletonServices.getInstance().getMongoClient().getDB(db));
					SingletonServices.getInstance().getMongoClient().setWriteConcern(WriteConcern.JOURNALED);
				}
			}
			int intPort = 5000;
			try {
				intPort = Integer.parseInt(port);
			} catch (Exception e) {
			}
			SingletonServices.getInstance().setMdsRpcServer(new JMQRPCServer(1));
			SingletonServices.getInstance().setMdsRpcClient(new JMQRPCClient(1));
			SingletonServices.getInstance().getMdsRpcClient().init(0);
			SingletonServices.getInstance().getMdsRpcClient().start();
			SingletonServices.getInstance().getMdsRpcServer().init(intPort);
			SingletonServices.getInstance().getMdsRpcServer().start();

			SingletonServices.getInstance().addHandler(CUQueryHandler.class);
			SingletonServices.getInstance().addHandler(DeviceQueyHandler.class);
			SingletonServices.getInstance().addHandler(PerformanceTest.class);
		} catch (Throwable e) {
			e.printStackTrace();
		}

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.ref.server.plugins.REFDeinitPlugin#customDeinit()
	 */
	public void customDeinit() {
		try {
			SingletonServices.getInstance().getMdsRpcServer().deinit();
			SingletonServices.getInstance().getMdsRpcClient().deinit();
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

}
