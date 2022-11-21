// Next.js API route support: https://nextjs.org/docs/api-routes/introduction
import { exec } from 'child_process';
import type { NextApiRequest, NextApiResponse } from 'next'
import shelljs from "shelljs"

export default function handler (
  req: NextApiRequest,
  res: NextApiResponse<any>
) {
  console.log(req.method);
  if (req.method == "GET") {
    res.status(400).send("114514");
  } else {
    const { timeLimit, memoryLimit, inputFile, outputFile, logFile } = req.query;
    exec(`${runnerPath} ${filePath} ${timeLimit} ${memoryLimit} ${inputFile} ${outputFile} ${logFile}`);
  }
}
