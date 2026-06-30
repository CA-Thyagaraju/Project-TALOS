from setuptools import find_packages, setup
from glob import glob
import os

package_name = 'talos_gazebo'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        (
            'share/ament_index/resource_index/packages',
            ['resource/' + package_name]
),
        (
            os.path.join('share', package_name),
            ['package.xml']
        ),
        (
            os.path.join('share', package_name, 'launch'),
            glob('launch/*.launch.py')
        ),
        (
            os.path.join('share', package_name, 'worlds'),
            glob('worlds/*')
        ),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='ca-thyagaraju',
    maintainer_email='cathyagaraju2000@gmail.com',
    description='Gazebo simulation package for Project TALOS',
    license='MIT',
    extras_require={
        'test': ['pytest'],
    },
    entry_points={
        'console_scripts': [],
    },
)