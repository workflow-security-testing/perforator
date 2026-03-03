CREATE SEQUENCE IF NOT EXISTS cluster_top_generations_id_seq OWNED BY cluster_top_generations.id;

ALTER TABLE cluster_top_generations ALTER COLUMN id SET DEFAULT nextval('cluster_top_generations_id_seq');

SELECT setval(
    'cluster_top_generations_id_seq', 
    COALESCE((SELECT MAX(id) FROM cluster_top_generations), 1)
);
