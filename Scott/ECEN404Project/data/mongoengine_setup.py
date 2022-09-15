import mongoengine


def global_initialization():
    mongoengine.register_connection(alias='core', name='NCID_local_DB')
    mongoengine.connect('NCID_local_DB', host='127.0.0.1', port=27017)
