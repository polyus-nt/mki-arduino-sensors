"""Standalone chemreactor sensor tests; no other repository required."""
import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default="g++")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix="chem-sensors-") as directory:
        for name in ("sensor_board_test", "bmp280_test", "dht11_test"):
            binary = Path(directory) / (name + ".exe")
            extra = ["-I", str(root / "tests/sensor-stubs")] if name == "sensor_board_test" else []
            subprocess.run([args.cxx, "-std=c++11", "-Wall", "-Wextra", *extra,
                            str(root / "tests" / (name + ".cpp")), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)


if __name__ == "__main__":
    main()
