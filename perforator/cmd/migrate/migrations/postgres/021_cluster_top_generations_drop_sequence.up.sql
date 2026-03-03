ALTER TABLE cluster_top_generations ALTER COLUMN id DROP DEFAULT;
DROP SEQUENCE IF EXISTS cluster_top_generations_id_seq;
