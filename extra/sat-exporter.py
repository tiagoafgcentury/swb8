#!/usr/bin/python3

import psycopg2
import json
import pandas as pd
import appdirs
import mysql.connector
import xlsxwriter

config_file = "{}/settings.json".format(appdirs.user_config_dir('sat-exporter'))
last_id = -1
registros_file_name = "{}/servicos.xlsx".format(appdirs.user_data_dir(
    'sat-exporter', 'Century'))

redmine_last_update = None

settings = dict()
settings_updated = False

try:
    with open(config_file) as file:
        settings = json.load(file)

    print(f"Using: {config_file}")

    if 'last_id' in settings:
        last_id = settings['last_id']

    if 'outfile' in settings:
        registros_file_name = settings['outfile']

    if 'redmine_last_update' in settings:
        redmine_last_update = settings['redmine_last_update']

except FileNotFoundError:
    print(f"{config_file} not found!")

except Exception as e:
    print(type(e))
    print(e)


# REDMINE
mydb = mysql.connector.connect(
  host="10.0.11.4",
  port=33061,
  user="root",
  password="o7i738g69j1l438UIerE0fnsgQE8qtqMTR0",
  database="redmine"
)

mycursor = mydb.cursor()

mycursor.execute("select unix_timestamp(updated_on) from issues order by updated_on desc limit 1")
rec = mycursor.fetchone()

if rec[0] != redmine_last_update:
    settings['redmine_last_update'] = rec[0]
    settings_updated = True

    mycursor.execute("""select
    CONCAT(u.firstname, ' ', u.lastname) as "User",
    p.name as "Projeto",
    i.id as "Issue",
    is2.name as "Status",
    i.subject as "Atividade",
    cv.value as "Sprint",
    i.start_date as "Inicio",
    i.due_date as "Fim",
    datediff(i.due_date, now()) as "Prazo (dias)",
    pri.name as "Prioridade",
    v.name as "Versão",
    i.done_ratio / 100.0
from issues i
    join users u on u.id = i.assigned_to_id
    join issue_statuses is2 on is2.id = i.status_id
    join projects p on p.id = i.project_id
    left join enumerations pri on pri.id = i.priority_id and pri.`type` = 'IssuePriority'
    left join custom_values cv on
        cv.customized_type = 'Issue'
        and cv.customized_id = i.id
        and cv.custom_field_id = (select id from custom_fields cf where name = 'Sprint')
    left join versions v on v.id = i.fixed_version_id
where p.status != 9
  and is2.is_closed = 0
order by p.name, u.firstname, pri.`position`""")

    workbook = xlsxwriter.Workbook('/home/gianni/Documentos/OneDrive/sat-monitor/redmine.xlsx')
    worksheet = workbook.add_worksheet('Issues')

    row = 0

    cell_format = workbook.add_format()
    cell_format.set_bold()
    cell_format.set_align('center')
    worksheet.write_string(row, 0, "User", cell_format)
    worksheet.write_string(row, 1, "Projeto", cell_format)
    worksheet.write_string(row, 2, "Issue", cell_format)
    worksheet.write_string(row, 3, "Status", cell_format)
    worksheet.write_string(row, 4, "Atividade", cell_format)
    worksheet.write_string(row, 5, "Sprint", cell_format)
    worksheet.write_string(row, 6, "Inicio", cell_format)
    worksheet.write_string(row, 7, "Fim", cell_format)
    worksheet.write_string(row, 8, "Prazo (dias)", cell_format)
    worksheet.write_string(row, 9, "Prioridade", cell_format)
    worksheet.write_string(row, 10, "Versão", cell_format)
    worksheet.write_string(row, 11, "Concluido", cell_format)

    percen_format = workbook.add_format({'num_format': '0%'})
    date_format = workbook.add_format({'num_format': 'yyyy-mm-dd'})

    for rec in mycursor.fetchall():
        row += 1
        worksheet.write_string(row, 0, rec[0] or '')
        worksheet.write_string(row, 1, rec[1] or '')
        worksheet.write_number(row, 2, rec[2] or 0)
        worksheet.write_string(row, 3, rec[3] or '')
        worksheet.write_string(row, 4, rec[4] or '')
        worksheet.write_string(row, 5, rec[5] or '')
        if rec[6]:
            worksheet.write_datetime(row, 6, rec[6], date_format)
        if rec[7]:
            worksheet.write_datetime(row, 7, rec[7], date_format)
        if rec[8]:
            worksheet.write_number(row, 8, rec[8] or '')
        worksheet.write_string(row, 9, rec[9] or '')
        worksheet.write_string(row, 10, rec[10] or '')
        worksheet.write_number(row, 11, rec[11] or 0, percen_format)

    worksheet.autofit()

    workbook.close()

# END REDMINE

con = psycopg2.connect(host='10.0.11.4', port=5440, database='portal',
                       user='postgres')

cur = con.cursor()

cur.execute('''select id_lineup from dvb.lineup l order by id_lineup desc
    limit 1''')
recset = cur.fetchall()
if recset[0][0] != last_id:
    last_id = recset[0][0]
    print(f"New Data: {recset[0][0]}/{last_id}")

    cur.execute('''select
    tsid, nid, onid, service_id, freq, sr, polarity,
    viewer_channel, name, bouquet_name , bouquet_id,
    video_codec, video_pid, pcr_pid, epg_pid,
    service_type, regionalizacao, zonas,
    audio_0_pid, audio_0_lang, audio_0_codec,
    audio_1_pid, audio_1_lang, audio_1_codec,
    audio_2_pid, audio_2_lang, audio_2_codec,
    audio_3_pid, audio_3_lang, audio_3_codec,
    audio_4_pid, audio_4_lang, audio_4_codec
from dvb.lineup_table''')
    recset = cur.fetchall()

    recset.insert(0, ["TSI", "Network ID", "Original Network ID", "Service ID",
                      "Frequência", "S/R", "Polaridade",
                      "Canal", "Nome do Serviço",  "Bouquet", "Bouquet ID",
                      "Video Codec", "Video PID", "PCR PID", "EPG PID",
                      "Tipo de Serviço", "Regionalização", "Zones",
                      "Audio 0 - PID", "Audio 0 - Lingua", "Audio 0 - Codec",
                      "Audio 1 - PID", "Audio 1 - Lingua", "Audio 1 - Codec",
                      "Audio 2 - PID", "Audio 2 - Lingua", "Audio 2 - Codec",
                      "Audio 3 - PID", "Audio 3 - Lingua", "Audio 3 - Codec"])

    # Create a Pandas dataframe from the data.
    df_lineup = pd.DataFrame(recset)

    cur.execute('''select freq, sr, polarity, viewer_channel, name,
    bouquet_name, video_codec,
    case service_type_id
        when 1 then 'TV'
        when 25 then 'TV'
        when 31 then 'TV - HVEC'
        when 2 then 'Radio'
        else service_type
    end as "service_type"
from dvb.lineup_table
where (not service_type_id in (0, 144)) and
    (bouquet_id = 0 or bouquet_id != 15000)
order by freq, viewer_channel, name''')
    recset = cur.fetchall()

    recset.insert(0, ["Frequência", "S/R", "Polaridade", "Canal",
                      "Nome do Serviço", "Bouquet", "Video Codec",
                      "Tipo de Serviço"])

    # Create a Pandas dataframe from the data.
    df_lineup_resumido = pd.DataFrame(recset)

    cur.execute('''select l.created_at, s.name, l.description
from public.logs l
    left join public.audio a on l."table" = 'audios' and l.item_id = a.id
    left join public.services s on (l."table" = 'services'
                                    and l.item_id = s.id)
                                    or (a.service_id = s.id)
where l.created_at > (now() - '48 hours'::interval)
    and l."table" != 'process'
order by created_at desc''')
    recset = cur.fetchall()

    recset.insert(0, ["Data/Hora", "Serviço", "Descrição"])

    # Create a Pandas dataframe from the data.
    df_logs = pd.DataFrame(recset)

    # cur.execute('select * from dvb.zones_imp')
    # recset = cur.fetchall()
    #
    # recset.insert(0, ["Zona", "Zone ID", "City ID", "Cidade", "Estado",
    #                   "Estado ID", "CEP Min.", "CEP Max"])
    #
    # df_zones_imp = pd.DataFrame(recset)

    # Create a Pandas Excel writer using XlsxWriter as the engine.
    writer = pd.ExcelWriter(registros_file_name, engine='xlsxwriter')

    # Convert the dataframe to an XlsxWriter Excel object.
    df_lineup.to_excel(writer, sheet_name='Serviços', header=False,
                       index=False)
    df_lineup_resumido.to_excel(writer, sheet_name='Serviços Resumido',
                                header=False, index=False)
    df_logs.to_excel(writer, sheet_name='Log de Alterações', header=False,
                     index=False)
    # df_zones_imp.to_excel(writer, sheet_name='Zonas', header=False,
    #                       index=False)

    # Close the Pandas Excel writer and output the Excel file.
    writer.close()

    con.close()

    settings['last_id'] = last_id
    settings_updated = True

    print(f"Updated: '{registros_file_name}'")

if settings_updated:
    try:
        with open(config_file, 'w') as outfile:
            outfile.write(json.dumps(settings))
    except OSError:
        print(f"Failed to save settings: {config_file}")
