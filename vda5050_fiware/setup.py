from setuptools import find_packages, setup

package_name = "vda5050_fiware"

setup(
    name=package_name,
    version="0.0.0",
    packages=[package_name],
    # packages=find_packages(exclude=['test']),
    data_files=[
        ("share/ament_index/resource_index/packages", ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="rosi",
    maintainer_email="glenn_tan@artc.a-star.edu.sg",
    description="TODO: Package description",
    license="TODO: License declaration",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "vda5050_fiware = vda5050_fiware.vda5050_fiware:main",
            "mock_agv = vda5050_fiware.mock_agv:main",
            "mock_order = vda5050_fiware.mock_order:main",
            "mock_update_order = vda5050_fiware.mock_update_order:main",
        ],
    },
)
