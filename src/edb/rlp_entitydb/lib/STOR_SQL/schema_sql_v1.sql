
CREATE TABLE edb_data_source (
    dts_rowid INTEGER PRIMARY KEY ASC,
    dts_uuid TEXT UNIQUE,
    dts_name TEXT,
    dts_type TEXT,
    dts_metadata TEXT,
    dts_schema TEXT,
    dts_origin INTEGER,
    dts_version INTEGER,
    dts_create_date INTEGER,
    dts_create_user TEXT,
    dts_update_date INTEGER,
    dts_update_user TEXT

);


CREATE TABLE edb_entity_type (
    et_rowid INTEGER PRIMARY KEY ASC,
    et_uuid TEXT UNIQUE,
    et_name TEXT,
    et_metadata TEXT,
    et_dts_uuid TEXT,
    et_parent_uuid TEXT,
    et_create_date INTEGER,
    et_create_user TEXT,
    et_update_date INTEGER,
    et_update_user TEXT


);


CREATE TABLE edb_field_type (
    ft_rowid INTEGER PRIMARY KEY ASC,
    ft_uuid TEXT UNIQUE,
    ft_name TEXT,
    ft_display_name TEXT,
    ft_type_name TEXT, // int, float, str, json, binary, entity
    ft_valtype INTEGER,
    ft_metadata TEXT,
    ft_et_uuid TEXT, // foreign key to edb_entity_type table
    ft_create_date INTGER,
    ft_create_user TEXT,
    ft_update_date INTEGER,
    ft_update_user TEXT

);


CREATE TABLE edb_entity (

    e_rowid INTEGER PRIMARY KEY ASC,
    e_uuid TEXT UNIQUE,
    e_name TEXT,
    e_metadata TEXT,
    e_et_uuid TEXT,
    e_create_date INTEGER,
    e_create_user TEXT,
    e_update_date INTEGER,
    e_update_user TEXT

);


CREATE TABLE edb_field_value (
    
    fv_rowid INTEGER PRIMARY KEY ASC,
    fv_uuid TEXT UNIQUE,
    fv_name TEXT,
    fv_metadata TEXT,
    fv_history TEXT, // history of changes to field value
    fv_e_uuid TEXT,
    fv_et_uuid TEXT,
    fv_f_uuid TEXT, // foreign key to field table
    fv_create_date INTEGER,
    fv_create_user TEXT,
    fv_update_date INTEGER,
    fv_update_user TEXT,
    fv_type_name TEXT, // str, int, float, etc
    fv_v_int INTEGER,
    fv_v_str TEXT,
    fv_v_float REAL,
    fv_v_json TEXT,
    fv_v_binary BLOB,
    
    fv_v_e_uuid TEXT,
    fv_v_et_uuid TEXT,
    fv_v_dts_uuid TEXT
    
);


CREATE TABLE edb_event (

    evt_rowid INTEGER PRIMARY KEY ASC,
    evt_uuid TEXT UNIQUE,
    evt_name TEXT,
    evt_e_uuid TEXT, // an entity uuid, can be used to reference this event
    evt_status TEXT,
    evt_data TEXT,
    evt_r_rowid INTEGER,
    evt_create_date INTEGER,
    evt_create_user TEXT,
    evt_update_date INTEGER,
    evt_update_user TEXT,

);

CREATE TABLE edb_event_queue (

    evtq_rowid INTEGER PRIMARY KEY ASC,
    evtq_uuid TEXT UNIQUE,
    evtq_name TEXT,
    evtq_data TEXT,
    evtq_create_date INTEGER,
    evtq_create_user TEXT,
    evtq_update_date INTEGER,
    evtq_update_user TEXT,

);


