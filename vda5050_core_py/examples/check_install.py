from rich.pretty import pprint

try:
    import vda5050_core_py as vda
    print("vda5050_core_py is installed ✅")
    
    # for k, v in vda.__dict__.items():
    #     pprint({k: v})

    # print(vda.__doc__)
    # print(vda.__dict__)

    # vda.create_default_mqtt_client()
except:
    print("vda5050_core_py is not installed 🚫")