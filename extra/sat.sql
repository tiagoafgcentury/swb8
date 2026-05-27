--
-- PostgreSQL database dump
--

-- Dumped from database version 14.5
-- Dumped by pg_dump version 14.5

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: sat; Type: DATABASE; Schema: -; Owner: postgres
--

ALTER DATABASE sat OWNER TO postgres;

\connect sat

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: dvb_mode; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.dvb_mode (
    dvb_mode_id integer NOT NULL,
    dvb_mode text NOT NULL
);


ALTER TABLE dvb.dvb_mode OWNER TO postgres;

--
-- Name: fec_rate; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.fec_rate (
    fec_rate_id integer NOT NULL,
    fec_rate text NOT NULL
);


ALTER TABLE dvb.fec_rate OWNER TO postgres;

--
-- Name: modulation_type; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.modulation_type (
    modulation_type_id integer NOT NULL,
    modulation_type text NOT NULL
);


ALTER TABLE dvb.modulation_type OWNER TO postgres;

--
-- Name: nit; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.nit (
    nit_id integer NOT NULL,
    lock_id integer,
    datetime timestamp without time zone DEFAULT now() NOT NULL,
    network_id integer,
    version_number smallint,
    current_next_indicator smallint,
    network_name text
);


ALTER TABLE dvb.nit OWNER TO postgres;

--
-- Name: nit_linkage_descriptors; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.nit_linkage_descriptors (
    nit_id integer NOT NULL,
    transport_stream_id integer NOT NULL,
    original_network_id integer,
    service_id integer,
    linkage_type integer
);


ALTER TABLE dvb.nit_linkage_descriptors OWNER TO postgres;

--
-- Name: nit_nit_id_seq; Type: SEQUENCE; Schema: dvb; Owner: postgres
--

ALTER TABLE dvb.nit ALTER COLUMN nit_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME dvb.nit_nit_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: nit_transport_streams; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.nit_transport_streams (
    nit_id integer NOT NULL,
    transport_stream_id integer NOT NULL,
    original_network_id integer,
    service_id integer,
    viewer_channel integer
);


ALTER TABLE dvb.nit_transport_streams OWNER TO postgres;

--
-- Name: pat; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.pat (
    lock_id integer NOT NULL,
    datetime timestamp without time zone DEFAULT now() NOT NULL,
    program integer NOT NULL,
    pid integer
);


ALTER TABLE dvb.pat OWNER TO postgres;

--
-- Name: transponders; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.transponders (
    frequency integer NOT NULL,
    symbol_rate integer NOT NULL,
    dvb_mode dvb.dvbmode NOT NULL,
    polarity dvb.polarity NOT NULL,
    active boolean DEFAULT true
);


ALTER TABLE dvb.transponders OWNER TO postgres;

--
-- Name: transponders_lock; Type: TABLE; Schema: dvb; Owner: postgres
--

CREATE TABLE dvb.transponders_lock (
    lock_id integer NOT NULL,
    frequency integer NOT NULL,
    datetime timestamp without time zone DEFAULT now() NOT NULL,
    success boolean NOT NULL,
    tuner_frequency integer,
    symbol_rate integer,
    modulation_type integer,
    fec_rate integer,
    signal_type integer,
    strength integer,
    quality integer
);


ALTER TABLE dvb.transponders_lock OWNER TO postgres;

--
-- Name: transponders_lock_lock_id_seq; Type: SEQUENCE; Schema: dvb; Owner: postgres
--

CREATE SEQUENCE dvb.transponders_lock_lock_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE dvb.transponders_lock_lock_id_seq OWNER TO postgres;

--
-- Name: transponders_lock_lock_id_seq; Type: SEQUENCE OWNED BY; Schema: dvb; Owner: postgres
--

ALTER SEQUENCE dvb.transponders_lock_lock_id_seq OWNED BY dvb.transponders_lock.lock_id;


--
-- Name: transponders_lock lock_id; Type: DEFAULT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.transponders_lock ALTER COLUMN lock_id SET DEFAULT nextval('dvb.transponders_lock_lock_id_seq'::regclass);


--
-- Data for Name: dvb_mode; Type: TABLE DATA; Schema: dvb; Owner: postgres
--

COPY dvb.dvb_mode (dvb_mode_id, dvb_mode) FROM stdin;
0	DVB
1	DVB2
\.


--
-- Data for Name: fec_rate; Type: TABLE DATA; Schema: dvb; Owner: postgres
--

COPY dvb.fec_rate (fec_rate_id, fec_rate) FROM stdin;
0	AUTO
1	FEC_1_2
2	FEC_2_3
3	FEC_3_4
4	FEC_4_5
5	FEC_5_6
6	FEC_6_7
7	FEC_7_8
8	FEC_8_9
9	FEC_9_10
10	FEC_1_4
11	FEC_1_3
12	FEC_2_5
13	FEC_3_5
14	FEC_5_9
15	FEC_7_9
16	FEC_4_15
17	FEC_7_15
18	FEC_8_15
19	FEC_11_15
20	FEC_13_18
21	FEC_9_20
22	FEC_11_20
23	FEC_23_36
24	FEC_25_36
25	FEC_11_45
26	FEC_13_45
27	FEC_14_45
28	FEC_26_45
29	FEC_28_45
30	FEC_29_45
31	FEC_31_45
32	FEC_32_45
33	FEC_77_90
\.


--
-- Data for Name: modulation_type; Type: TABLE DATA; Schema: dvb; Owner: postgres
--

COPY dvb.modulation_type (modulation_type_id, modulation_type) FROM stdin;
0	Default
1	QAM_4
2	QAM_4_NR
3	QAM_16
4	QAM_32
5	QAM_64
6	QAM_128
7	QAM_256
8	QAM_512
9	BPSK
10	QPSK
11	DQPSK
12	PSK_8
13	APSK_16
14	APSK_32
15	APSK_64
16	APSK_128
17	APSK_256
18	APSK_L_8
19	APSK_L_16
20	APSK_L_32
21	APSK_L_64
22	APSK_L_128
23	APSK_L_256
24	VSB_8
25	VSB_16
26	AUTO
\.

--
-- Data for Name: transponders; Type: TABLE DATA; Schema: dvb; Owner: postgres
--

COPY dvb.transponders (frequency, symbol_rate, dvb_mode, polarity, active) FROM stdin;
12120000	29900	DVB2	Vertical	t
11740000	34300	DVB2	Horizontal	t
12160000	29900	DVB2	Vertical	t
12420000	29900	DVB2	Horizontal	t
\.


--
-- Name: dvb_mode dvb_mode_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.dvb_mode
    ADD CONSTRAINT dvb_mode_pkey PRIMARY KEY (dvb_mode_id);


--
-- Name: fec_rate fec_rate_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.fec_rate
    ADD CONSTRAINT fec_rate_pkey PRIMARY KEY (fec_rate_id);


--
-- Name: modulation_type modulation_type_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.modulation_type
    ADD CONSTRAINT modulation_type_pkey PRIMARY KEY (modulation_type_id);


--
-- Name: nit_linkage_descriptors nit_linkage_descriptors_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.nit_linkage_descriptors
    ADD CONSTRAINT nit_linkage_descriptors_pkey PRIMARY KEY (nit_id, transport_stream_id);


--
-- Name: nit nit_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.nit
    ADD CONSTRAINT nit_pkey PRIMARY KEY (nit_id);


--
-- Name: nit_transport_streams nit_transport_streams_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.nit_transport_streams
    ADD CONSTRAINT nit_transport_streams_pkey PRIMARY KEY (nit_id, transport_stream_id);


--
-- Name: pat pat_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.pat
    ADD CONSTRAINT pat_pkey PRIMARY KEY (lock_id, program);


--
-- Name: transponders_lock transponders_lock_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.transponders_lock
    ADD CONSTRAINT transponders_lock_pkey PRIMARY KEY (lock_id);


--
-- Name: transponders transponders_pkey; Type: CONSTRAINT; Schema: dvb; Owner: postgres
--

ALTER TABLE ONLY dvb.transponders
    ADD CONSTRAINT transponders_pkey PRIMARY KEY (frequency);


--
-- Name: nit_datetime_idx; Type: INDEX; Schema: dvb; Owner: postgres
--

CREATE INDEX nit_datetime_idx ON dvb.nit USING btree (datetime);


--
-- Name: nit_lock_id_idx; Type: INDEX; Schema: dvb; Owner: postgres
--

CREATE INDEX nit_lock_id_idx ON dvb.nit USING btree (lock_id);


--
-- Name: nit_network_idx; Type: INDEX; Schema: dvb; Owner: postgres
--

CREATE INDEX nit_network_idx ON dvb.nit USING btree (network_id);


--
-- Name: pat_datetime_idx; Type: INDEX; Schema: dvb; Owner: postgres
--

CREATE INDEX pat_datetime_idx ON dvb.pat USING btree (datetime);


--
-- Name: pat_lock_id_idx; Type: INDEX; Schema: dvb; Owner: postgres
--

CREATE INDEX pat_lock_id_idx ON dvb.pat USING btree (lock_id);


--
-- Name: transponders_lock_frequency_idx; Type: INDEX; Schema: dvb; Owner: postgres
--

CREATE INDEX transponders_lock_frequency_idx ON dvb.transponders_lock USING btree (frequency);


--
-- Name: transponders_lock_success_idx; Type: INDEX; Schema: dvb; Owner: postgres
--

CREATE INDEX transponders_lock_success_idx ON dvb.transponders_lock USING btree (success);


--
-- Name: TABLE nit; Type: ACL; Schema: dvb; Owner: postgres
--

GRANT ALL ON TABLE dvb.nit TO sat;


--
-- Name: TABLE nit_linkage_descriptors; Type: ACL; Schema: dvb; Owner: postgres
--

GRANT ALL ON TABLE dvb.nit_linkage_descriptors TO sat;


--
-- Name: TABLE nit_transport_streams; Type: ACL; Schema: dvb; Owner: postgres
--

GRANT ALL ON TABLE dvb.nit_transport_streams TO sat;


--
-- Name: TABLE pat; Type: ACL; Schema: dvb; Owner: postgres
--

GRANT ALL ON TABLE dvb.pat TO sat;


--
-- Name: TABLE transponders; Type: ACL; Schema: dvb; Owner: postgres
--

GRANT SELECT ON TABLE dvb.transponders TO sat;


--
-- Name: TABLE transponders_lock; Type: ACL; Schema: dvb; Owner: postgres
--

GRANT SELECT,INSERT ON TABLE dvb.transponders_lock TO sat;


--
-- Name: SEQUENCE transponders_lock_lock_id_seq; Type: ACL; Schema: dvb; Owner: postgres
--

GRANT ALL ON SEQUENCE dvb.transponders_lock_lock_id_seq TO sat;


--
-- PostgreSQL database dump complete
--

