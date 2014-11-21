package it.infn.chaos.mds.rpc.server;

import java.util.Hashtable;

import it.infn.chaos.mds.RPCConstants;

import org.bson.BasicBSONObject;
import org.ref.common.helper.ExceptionHelper;
import org.zeromq.ZContext;
import org.zeromq.ZMQ.Socket;

import zmq.ZMQ;

public class JMQRPCClient extends RPCClient {
	private ZContext						context				= null;
	private int								threadNUmber		= 0;
	private Hashtable<String, SocketSlot>	serverSlotHashtable	= new Hashtable<String, JMQRPCClient.SocketSlot>();

	public JMQRPCClient(int threadNUmber) {
		this.threadNUmber = threadNUmber;
	}

	@Override
	public void init(int port) throws Throwable {
		context = new ZContext(this.threadNUmber);
	}

	@Override
	public void start() throws Throwable {

	}

	@Override
	public void deinit() throws Throwable {
		context.destroy();

	}

	private SocketSlot getSocketForADDR(String serverAddress) {
		synchronized (serverSlotHashtable) {
			if (serverSlotHashtable.containsKey(serverAddress)) {
				return serverSlotHashtable.get(serverAddress);
			}else {
				SocketSlot socketSlot = new SocketSlot();
				socketSlot.socket = context.createSocket(ZMQ.ZMQ_REQ);
				socketSlot.socket.setSendTimeOut(5000);
				socketSlot.socket.setReceiveTimeOut(5000);
				socketSlot.socket.setSndHWM(30);
				socketSlot.socket.setLinger(1000);
				socketSlot.socket.connect(serverAddress);
				serverSlotHashtable.put(serverAddress, socketSlot);
				return socketSlot;
			}
		}
	}

	@Override
	public void sendMessage(BasicBSONObject messageData) throws Throwable {
		
		String serverAddress = "tcp://" + (messageData.containsField(RPCConstants.CS_CMDM_REMOTE_HOST_IP) ? messageData.getString(RPCConstants.CS_CMDM_REMOTE_HOST_IP) : null);
		SocketSlot socketSlot = getSocketForADDR(serverAddress);
		synchronized (socketSlot) {
			if(socketSlot.inError) {
				System.out.println("socke in error got new one for " + serverAddress);
				socketSlot = getSocketForADDR(serverAddress);
			}
			try {
				byte[] rawData = encoder.encode(messageData);

				if (!socketSlot.socket.send(rawData)) {
					throw new Throwable("Error on sending to "+serverAddress);
				}

				if ((rawData = socketSlot.socket.recv()) == null) {
					throw new Throwable("Error on receiving to "+serverAddress);
				}
				BasicBSONObject bsonResult = (BasicBSONObject) decoder.readObject(rawData);
				//System.out.println("Submission result->" + bsonResult);
			} catch (Throwable e) {
				System.out.println("Exception->" + ExceptionHelper.getInstance().printExceptionStack(e));
				synchronized (serverSlotHashtable) {
					try {
						socketSlot.inError = true;
						socketSlot.socket.close();
					} catch (Exception e2) {
					}
					System.out.println("Removing slot from hash table for "+serverAddress);
					serverSlotHashtable.remove(serverAddress);
				}
				throw e;
			} finally {
			}
		}
	}

	class SocketSlot {
		Socket	socket;
		boolean inError = false;
	}

}
