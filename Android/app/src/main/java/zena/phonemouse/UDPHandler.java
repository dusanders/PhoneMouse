package zena.phonemouse;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;

/**
 * Created by Zena on 8/14/2015.
 */
public class UDPHandler {
    private DatagramSocket socket;
    private DatagramPacket packet;
    private InetAddress serverAddress;
    private int serverPort;
    public UDPHandler(InetAddress passedServer, int passedPort) throws SocketException {
        serverAddress = passedServer;
        serverPort = passedPort;
        socket = new DatagramSocket(passedPort);
    }
    public void disconnect() {
        if(socket.isConnected()) {
            socket.disconnect();
            socket.close();
        }
    }
    public void connect() throws SocketException {
        if(!socket.isConnected()) {
            socket.connect(new InetSocketAddress(serverAddress, serverPort));
        }
    }
    public void udpSend(DatagramPacket passedPacket) throws IOException {
        packet = passedPacket;
        socket.send(packet);
    }
}
