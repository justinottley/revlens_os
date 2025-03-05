

class EdbLocalClient(object):

    def __init__(self, cntrl):
        self.__cntrl = cntrl
    
    @property
    def cntrl(self):
        return self.__cntrl

    def get_entity_types(self):
        return self.cntrl.getEntityTypes()

    def create_entity_type(self, etype_name, parent=''):
        if parent is None:
            parent = ''
        return self.cntrl.createEntityType(etype_name, parent)

    def get_entity_type_fields(self, etype_name, parents=True):
        return self.cntrl.getEntityTypeFields(etype_name, parents)

    def create_entity_type_field(self, entity_name, field_name, display_name, field_type, val_type, metadata=None):
        
        if not metadata:
            metadata = {}

        return self.cntrl.createEntityTypeField(
            entity_name,
            field_name,
            display_name,
            field_type,
            val_type,
            metadata
        )

    def create(self, e_type, name, fields, uuid=""):
        return self.cntrl.create(e_type, name, fields, uuid)