# wss_echo_server.py
import asyncio
import websockets
import ssl

# Create an SSL context and load your certificate and private key
ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ssl_context.load_cert_chain(certfile="cert.pem", keyfile="key.pem")

# WebSocket handler that sends messages periodically and echoes received ones
async def echo(websocket):
    async def sender():
        count = 1
        while True:
            message = f"Server push message {count}"
            await websocket.send(message)
            print(f"Sent: {message}")
            count += 1
            await asyncio.sleep(5)  # Send every 5 seconds

    async def receiver():
        async for message in websocket:
            print(f"Received: {message}")
            await websocket.send(f"Echo: {message}")

    # Run sender and receiver concurrently
    await asyncio.gather(sender(), receiver())

# Main function to start the server
async def main():
    async with websockets.serve(echo, "localhost", 8766, ssl=ssl_context):
        print("Secure WebSocket server started at wss://localhost:8766")
        await asyncio.Future()  # Run forever

# Run the server
asyncio.run(main())
