from flask import Flask, render_template
import csv
import os

app = Flask(__name__)

CSV_FILE = "ping_report.csv"


@app.route("/")
def index():
    results = {}

    if os.path.exists(CSV_FILE):
        with open(CSV_FILE, "r", encoding="utf-8") as f:
            reader = csv.DictReader(f)

            for row in reader:
                ip = row["IP"]
                results[ip] = row

    alerts = []

    for ip, row in results.items():
        if row["状態"] == "NG":
            alerts.append(ip)

    return render_template(
        "index.html",
        results=results,
        alerts=alerts
    )


if __name__ == "__main__":
    app.run(debug=True)