GO_LIBRARY()

LICENSE(MIT)

VERSION(v0.4.48)

SRCS(
    addoffsetstotxn.go
    addpartitionstotxn.go
    address.go
    alterclientquotas.go
    alterconfigs.go
    alterpartitionreassignments.go
    alteruserscramcredentials.go
    apiversions.go
    balancer.go
    batch.go
    buffer.go
    client.go
    commit.go
    compression.go
    conn.go
    consumergroup.go
    crc32.go
    createacls.go
    createpartitions.go
    createtopics.go
    deleteacls.go
    deletegroups.go
    deletetopics.go
    describeacls.go
    describeclientquotas.go
    describeconfigs.go
    describegroups.go
    describeuserscramcredentials.go
    dialer.go
    discard.go
    electleaders.go
    endtxn.go
    error.go
    fetch.go
    findcoordinator.go
    groupbalancer.go
    heartbeat.go
    incrementalalterconfigs.go
    initproducerid.go
    joingroup.go
    kafka.go
    leavegroup.go
    listgroups.go
    listoffset.go
    listpartitionreassignments.go
    logger.go
    message.go
    message_reader.go
    metadata.go
    offsetcommit.go
    offsetdelete.go
    offsetfetch.go
    produce.go
    protocol.go
    rawproduce.go
    read.go
    reader.go
    record.go
    recordbatch.go
    resolver.go
    resource.go
    saslauthenticate.go
    saslhandshake.go
    sizeof.go
    stats.go
    syncgroup.go
    time.go
    transport.go
    txnoffsetcommit.go
    write.go
    writer.go
)

GO_TEST_SRCS(
    # addoffsetstotxn_test.go
    # addpartitionstotxn_test.go
    # address_test.go
    # alterclientquotas_test.go
    # alterconfigs_test.go
    # alterpartitionreassignments_test.go
    # alteruserscramcredentials_test.go
    # apiversions_test.go
    # balancer_test.go
    # batch_test.go
    builder_test.go
    # client_test.go
    # commit_test.go
    # conn_test.go
    # consumergroup_test.go
    # crc32_test.go
    # createacls_test.go
    # createpartitions_test.go
    # createtopics_test.go
    # deleteacls_test.go
    # deletegroups_test.go
    # deletetopics_test.go
    # describeacls_test.go
    # describeconfigs_test.go
    # describegroups_test.go
    # describeuserscramcredentials_test.go
    # dialer_test.go
    # discard_test.go
    # electleaders_test.go
    # error_test.go
    # example_groupbalancer_test.go
    # fetch_test.go
    # findcoordinator_test.go
    # groupbalancer_test.go
    # heartbeat_test.go
    # incrementalalterconfigs_test.go
    # initproducerid_test.go
    # joingroup_test.go
    # kafka_test.go
    # leavegroup_test.go
    # listgroups_test.go
    # listoffset_test.go
    # listpartitionreassignments_test.go
    # message_test.go
    # metadata_test.go
    # offsetcommit_test.go
    # offsetdelete_test.go
    # offsetfetch_test.go
    # produce_test.go
    # protocol_test.go
    # rawproduce_test.go
    # read_test.go
    # reader_test.go
    # resource_test.go
    # saslauthenticate_test.go
    # saslhandshake_test.go
    # syncgroup_test.go
    # transport_test.go
    # txnoffsetcommit_test.go
    # write_test.go
    # writer_test.go
)

GO_XTEST_SRCS(
    example_consumergroup_test.go
    example_writer_test.go
)

END()

RECURSE(
    compress
    gotest
    gzip
    lz4
    protocol
    sasl
    snappy
    testing
    topics
    zstd
)
