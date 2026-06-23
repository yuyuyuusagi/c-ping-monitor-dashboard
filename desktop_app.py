import threading
import webview
from app import app

def run_flask():
    app.run(host="127.0.0.1", port=5000, debug=False)

if __name__ == "__main__":
    flask_thread = threading.Thread(target=run_flask)
    flask_thread.daemon = True
    flask_thread.start()

    webview.create_window(
        "Ping Monitor Dashboard",
        "http://127.0.0.1:5000",
        width=1000,
        height=700
    )
    webview.start()