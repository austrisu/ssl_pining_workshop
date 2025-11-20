from http.server import HTTPServer, BaseHTTPRequestHandler
import ssl
# openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/CN=localhost"


class HelloHandler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"  # be explicit

    def do_GET(self):
        if self.path == "/secret":
            body = b"Here_you_go,_the_big_secret\n"
            # print(str(len(body)))
            self.send_response(200)
            self.send_header("Content-Type", "text/plain; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Connection", "close")
            self.end_headers()

            self.wfile.write(body)
            self.wfile.flush()
        else:
            self.send_error(404, "Not Found")


def run():
    server_address = ("127.0.0.1", 1337)  # https://127.0.0.1:1337/
    httpd = HTTPServer(server_address, HelloHandler)

    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile="cert.pem", keyfile="key.pem")

    httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

    print("Serving on https://127.0.0.1:1337/")
    httpd.serve_forever()


if __name__ == "__main__":
    run()
