"""_summary_"""
# Copyright 2025 HarvestX Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import time

import rclpy
from rclpy.node import Node


class StressLogger(Node):
    """_summary_

    Args:
        Node (_type_): _description_
    """

    def __init__(self):
        super().__init__('stress_logger')
        self.counter = 0

    def run(self, rate_hz=1000):
        """_summary_

        Args:
            rate_hz (int, optional): _description_. Defaults to 1000.
        """
        interval = 1.0 / rate_hz
        while rclpy.ok():
            msg = f"Stress test log message {self.counter}"
            level = self.counter % 5

            # ログレベルに応じた出力
            if level == 0:
                self.get_logger().debug(msg)
            elif level == 1:
                self.get_logger().info(msg)
            elif level == 2:
                self.get_logger().warning(msg)
            elif level == 3:
                self.get_logger().error(msg)
            else:
                self.get_logger().fatal(msg)

            self.counter += 1
            time.sleep(interval)


def main():
    """_summary_"""
    rclpy.init()

    # get_logger() を呼ぶ前にログレベルを設定
    rclpy.logging.set_logger_level('stress_logger', rclpy.logging.LoggingSeverity.DEBUG)

    node = StressLogger()
    try:
        node.run(rate_hz=500)  # 出力レートは必要に応じて調整
    except KeyboardInterrupt:
        pass
    rclpy.shutdown()


if __name__ == '__main__':
    main()
