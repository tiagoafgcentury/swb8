#!/bin/bash

# echo '10.0.11.4:5440:portal:build:QiObUVj3n2DpLQiIGg/vfEAh3kYtV8ASM22BKS/GHL8' >> ~/.pgpass

set -e

SQL=$(psql -h 10.0.11.4 -p 5440 -d portal -U build -w -t <<__EOF
select jsonb_agg(r.*)
from
(
    select network_id, tsid, frequency, polarity, jsonb_agg(obj) services
    from
    (
        select t.network_id, t.tsid, t.frequency, t.polarity, jsonb_build_object('svcid', s.svcid , 'catid', r.id, 'name', s.name, 'cat', r.description) obj
        from rates r
            join sat.services s on s.category_id = r.id
            join sat.transponders t on s.transponder_id = t.id
        where r.description != 'Novos canais'
    ) qry
    group by network_id, tsid, frequency, polarity
    order by network_id, tsid, frequency, polarity
) r
__EOF
) || exit 0

COUNT=$(echo "$SQL" | jq 'length');
END=$(echo "$COUNT" - 1 | bc)

echo "#pragma once"
echo "#define MBGUI_HAS_DYNAMIC_SERVICE_MAP $COUNT"

for i in $(seq 0 "$END" ); do
    SATID=$(echo "$SQL" | jq ".[$i].network_id")
    TSID=$(echo "$SQL" | jq ".[$i].tsid")
    FREQ=$(echo "$SQL" | jq ".[$i].frequency")
    POL=$(echo "$SQL" | jq ".[$i].polarity")

    echo -e "s_service_category_map[$SATID][$TSID] = { // $FREQ/$POL"

    SERVICES=$(echo "$SQL" | jq ".[$i].services")
    SVC_COUNT=$(echo "$SERVICES" | jq 'length');
    SVC_END=$(echo "$SVC_COUNT" - 1 | bc)

    for j in $(seq 0 "$SVC_END" ); do
        SVCID=$(echo "$SERVICES" | jq ".[$j].svcid")
        NAME=$(echo "$SERVICES" | jq ".[$j].name")
        CAT=$(echo "$SERVICES" | jq ".[$j].cat")
        CATID=$(echo "$SERVICES" | jq ".[$j].catid")
        echo "    {$SVCID, static_cast<Service_Category>($CATID)}, // $NAME - $CAT"
    done

    echo -e "};"
done
