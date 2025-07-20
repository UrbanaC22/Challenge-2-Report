from setuptools import find_packages, setup

package_name = 'data_and_nav'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='urbana',
    maintainer_email='urbana@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'hazard=data_and_nav.hazard:main',
            'dummy_data=data_and_nav.dummy_data:main',
        ],
    },
)
