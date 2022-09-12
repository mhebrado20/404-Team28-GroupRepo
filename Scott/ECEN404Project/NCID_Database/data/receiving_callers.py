import mongoengine


class ReceivingCaller(mongoengine.EmbeddedDocument):
    # voice signature of receiving caller
    voice_signature_id = mongoengine.EmbeddedDocumentField()
    voice_signature_name = mongoengine.StringField(default='John Doe')
    # will probably need to be sent by a function that analyzes the voice from the recording
    # an identifier corresponding to the voice signature for easier look up
    # will be a mongoengine embedded document
    meta = {
        'db_alias:': 'core',
        'collection': 'receiving callers'
    }
