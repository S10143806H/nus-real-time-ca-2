##1. Did the CA1 design document provide you a clear understanding of the intended design? Would you be able to implement the system based on the design document? (It may be worth attempting implementation to properly answer this question, although you are not required to submit any code that you attempt).

Lukas's design document provided sufficient details which made clear the intended design of real-time apple processing system. The main diagram did an excellent job separating out the three threads of processing. The flow of data was a particular strong point, showing the structure of data (any developer could clearly interpret the structure needed to support the data flowing through the messaging queues) and how the data would move (using domain-specific terminology like messaging queues makes the expectations on implementation clear as well).  In addition, the accompanying paragraphs in the report were concise without loss of logical clarity; they explained the crucial logic related to timing for the three main processes.

I will say, however, that additional design diagrams would have made the process easier for a potential developer to implement this system.  The crux of the system design was not detailed in diagrammatic form, which might have led improper interpretation of the design during implementation.

![ADDITIONAL DIAGRAM SUGGESTION MADE BY ME](path/to/where.png)

##2. What are the advantages and disadvantages of the CA1 design adopted? How did it differ from your own CA1 design. What improvements would you suggest to the CA1 design, if any?

The design proposed by Lukas is quite similar to mine;  this may be related to the fact that we collaborated on CA1. The use of three threads/processes, as well as message queues to communicate between them, was the core similarity. The way Lukas's design handles the logic of discarding apples after 5 seconds have passed is different.  His design states that he discards apples that have not been processed after 5 seconds.  In contrast, my design has no mechanism by which apples which are still being processed can be discarded.  Subsequent apples which have been blocked by previous apple processing are moved to the quality message queue immediately, marking them as UNKNOWN, and the actuator discards these UNKNOWN apples.

##3. Was the CA1 implementation in accordance with the design?

Lukas's CA1 implementation slightly differed to the proposed design.  The detail in his design about discarding apples that have taken longer than 5 seconds to be processed does not get fully implemented!  To his implementation's defense, the structure of three threads, two message queues in between, and most of the timing logic located in the actuator thread follows closely to the design diagram.  The discrepancy may be due to the fact that the diagram, as is, does not satisfy the design intention of discarding apples that are still being processed by the `process_photo` function after 5 seconds have elapsed.

![MAYBE SAME ADDITIONAL DIAGRAM SUGGESTION MADE BY ME](path/to/where.png)
This is where I should do a DFD of the Process Photo Thread.  Where there is concurrent logic going on, where the message gets sent to the actuator message buffer if 5 seconds has already passed with an UNKNOWN quality tag. Otherwise, send the quality tag which the `process_photo` function has returned.

With the current implementation, the `process_photo` function blocks the do-loop in the `runProcess` method;  since the passing of the message to the quality message queue comes below the `process_photo` function.

##4. Were there any notable features or shortcomings that you noticed in the CA1 implementation? What improvements would you suggest to the CA1 implementation, if any?

Try running the CA1 code provided by your peer under different test conditions. With virtual
machines (VM), you may simulate test conditions by changing the VM parameters. You may also
simulate test conditions by running other applications to load your computer and see how the
performance varies. Document your findings in the report:

##5. Summarize the performance of the CA1 code under different conditions (document the relevant conditions). Can you explain your the observed variability in performance in terms of the design or implementation decisions? Do you think there is scope for improvement in performance? In case you test any of your recommended changes to the design/code, you may wish to include code snippets and performance improvement details in an appendix to your report (appendix does not count towards the page limit). This is optional, but encouraged, as it will allow you to test your ideas, and enhance your learning experience.