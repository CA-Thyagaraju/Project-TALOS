import os
from glob import glob
from setuptools import find_packages, setup

package_name = 'robot_arm_urdf'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        # Register the package with the ROS2 resource index
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        # Install the package.xml file
        ('share/' + package_name, ['package.xml']),
        
        # Loop-equivalent: Install all contents of your SolidWorks asset directories
        (os.path.join('share', package_name, 'config'), glob(os.path.join('config', '*'))),
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*'))),
        (os.path.join('share', package_name, 'meshes'), glob(os.path.join('meshes', '*'))),
        (os.path.join('share', package_name, 'urdf'), glob(os.path.join('urdf', '*'))),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='ca-thyagaraju',
    maintainer_email='cathyagaraju2000@gmail.com',
    description='Python setup for SolidWorks exported URDF package',
    license='Apache-2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        ],
    },
)
