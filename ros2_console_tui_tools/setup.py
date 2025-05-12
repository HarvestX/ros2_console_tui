"""Setup."""
from setuptools import setup

package_name = 'ros2_console_tui_tools'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Yuki Yamamoto',
    maintainer_email='yuki8mamo10.hu@gmail.com',
    description='test tools',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            # Format: '<executable name> = <package>.<module>:<function>'
            'stress_log_publisher = ros2_console_tui_tools.stress_log_publisher:main',
            # Example for additional nodes:
            # 'another_node = ros2_console_tui_tools.another_module:main',
        ],
    },
)