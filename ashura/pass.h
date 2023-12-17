void init(Graph &graph, CmdBuffer &cmd_buffer)
{
  // SETUP
  //
  // get the number maximum number of offscreen draw passes in the scene = N
  //
  // create N color output render targets with undefined layout
  // optionally create N depth stencil output render targets with undefined
  // layout left to the pipeline to determine the inputs???
  //
}

void execute(Graph &graph, CmdBuffer &cmd_buffer)
{
  //
  // for all N outputs insert barrier to convert from used or newly created
  // layout to color attachment output layout
  //
  // for each N batch:
  //
  // for each z-sorted offscreen render pass:
  //
  //
  // RENDER
  // perform all intermediate rendering operations
  //
  // transition layout of color render target to shader read or transfer src
  // or dst
  //
  // render to target
  // insert barrier to convert layout back to color attachment output
  //
  // we might want to leave the final image layout or state until completion
  // of the pipeline as we don't know how exactly the will be used
  //
}